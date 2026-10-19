/* xmi2smf — Miles XMIDI (FORM/XDIR/CAT/EVNT) to Standard MIDI File.
 *
 * XMI timing is a fixed 120 Hz tick; an EVNT event is a run of delay
 * bytes (each < 0x80, summed) followed by one verbatim MIDI event.
 * The converter emits SMF format 0 with division=120 TPQN and a fixed
 * 1,000,000 us/qn tempo, which makes 1 SMF tick == 1 XMI tick exactly.
 * Embedded XMI tempo/meta events that would rescale the tick are
 * dropped (XMI tempo bytes do not change its fixed-rate clock).
 *
 * TIMB chunks map program numbers -> GM patches; applied to 0xCn events.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint32_t be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

typedef struct { uint8_t* p; size_t n, cap; } Buf;
static int bput(Buf* b, const void* d, size_t n) {
    if (b->n + n > b->cap) {
        size_t nc = (b->cap ? b->cap * 2 : 4096);
        while (nc < b->n + n) nc *= 2;
        uint8_t* np = (uint8_t*)realloc(b->p, nc);
        if (!np) return -1;
        b->p = np; b->cap = nc;
    }
    memcpy(b->p + b->n, d, n); b->n += n;
    return 0;
}
static int bvar(Buf* b, uint32_t v) {          /* SMF variable-length */
    uint8_t t[5]; int n = 0;
    t[n++] = (uint8_t)(v & 0x7F); v >>= 7;
    while (v) { t[n++] = (uint8_t)(0x80 | (v & 0x7F)); v >>= 7; }
    while (n--) if (bput(b, &t[n], 1)) return -1;
    return 0;
}

/* XMI quirk: 0x9n note-ons carry a trailing varlen duration and no
 * explicit note-off — the sequencer releases the note at on+dur.
 * We materialize those as real 0x8n events, so pending offs are kept
 * in a small min-heap and flushed (<= current tick) before each emit. */
typedef struct { uint32_t t; uint8_t ch, note; } OffNote;

typedef struct { OffNote* v; size_t n, cap; } OffHeap;
static int off_push(OffHeap* h, uint32_t t, uint8_t ch, uint8_t note) {
    if (h->n == h->cap) {
        size_t nc = h->cap ? h->cap * 2 : 64;
        OffNote* np = (OffNote*)realloc(h->v, nc * sizeof *np);
        if (!np) return -1;
        h->v = np; h->cap = nc;
    }
    size_t c = h->n++;
    while (c && h->v[(c - 1) / 2].t > t) {
        h->v[c] = h->v[(c - 1) / 2];
        c = (c - 1) / 2;
    }
    h->v[c].t = t; h->v[c].ch = ch; h->v[c].note = note;
    return 0;
}
static uint32_t off_min(const OffHeap* h) { return h->n ? h->v[0].t : ~0u; }
static OffNote off_pop(OffHeap* h) {
    OffNote top = h->v[0];
    OffNote last = h->v[--h->n];
    size_t p = 0;
    for (;;) {                          /* sift 'last' down */
        size_t l = 2 * p + 1, r = l + 1, s = p;
        if (l < h->n && h->v[l].t < last.t) s = l;
        if (r < h->n && h->v[r].t < (s == l ? h->v[l].t : last.t)) s = r;
        if (s == p) break;
        h->v[p] = h->v[s]; p = s;
    }
    if (h->n) h->v[p] = last;
    return top;
}
static void off_free(OffHeap* h) { free(h->v); h->v = NULL; h->n = h->cap = 0; }

/* Convert one EVNT stream into an MTrk body (no chunk header). */
static int conv_evnt(const uint8_t* ev, size_t n, const uint8_t timb[128],
                     Buf* trk) {
    static const uint8_t tempo[6] = {0xFF, 0x51, 0x03, 0x0F, 0x42, 0x40};
    static const uint8_t eot[4]   = {0x00, 0xFF, 0x2F, 0x00};
    OffHeap offs = {0};
    size_t i = 0;
    uint32_t now = 0;
    uint32_t lastEmit = 0;
    int rc = -1;
    if (bvar(trk, 0) || bput(trk, tempo, sizeof tempo)) return -1;

    /* emit the min pending note-off; bvar delta is relative to last emit */
    #define FLUSH_OFF() do { \
        OffNote o_ = off_pop(&offs); \
        if (bvar(trk, o_.t - lastEmit)) goto out; \
        uint8_t msg_[3] = {(uint8_t)(0x80 | o_.ch), o_.note, 0x40}; \
        if (bput(trk, msg_, 3)) goto out; \
        lastEmit = o_.t; \
    } while (0)
    /* drain offs at-or-before 'upto' (empty-heap min is ~0u — guard first) */
    #define FLUSH_UNTIL(upto) do { \
        while (offs.n && off_min(&offs) <= (upto)) FLUSH_OFF(); \
    } while (0)

    while (i < n) {
        uint32_t delta = 0;
        while (i < n && ev[i] < 0x80) delta += ev[i++];
        if (i >= n) break;
        now += delta;
        uint8_t st = ev[i];
        uint8_t type = st & 0xF0;

        if (st == 0xFF) {                    /* meta event */
            if (i + 2 > n) goto out;
            uint8_t mt = ev[i + 1];
            size_t j = i + 2;
            uint32_t len = 0; int lb = 0;
            do {
                if (j >= n) goto out;
                len = (len << 7) | (ev[j] & 0x7F);
            } while ((ev[j++] & 0x80) && ++lb < 4);
            if (j + len > n) goto out;
            if (mt == 0x51 || mt == 0x2F) {  /* tempo/eot: XMI-fixed clock */
                i = j + len;
                continue;
            }
            FLUSH_UNTIL(now);
            uint8_t hdr[2] = {0xFF, mt};
            if (bvar(trk, now - lastEmit) || bput(trk, hdr, 2) ||
                bvar(trk, len) || bput(trk, ev + j, len)) goto out;
            lastEmit = now;
            i = j + len;
        } else if (st == 0xF0 || st == 0xF7) {           /* sysex */
            size_t j = i + 1;
            uint32_t len = 0; int lb = 0;
            do {
                if (j >= n) goto out;
                len = (len << 7) | (ev[j] & 0x7F);
            } while ((ev[j++] & 0x80) && ++lb < 4);
            if (j + len > n) goto out;
            FLUSH_UNTIL(now);
            if (bvar(trk, now - lastEmit) || bput(trk, &st, 1) ||
                bvar(trk, len) || bput(trk, ev + j, len)) goto out;
            lastEmit = now;
            i = j + len;
        } else {                             /* channel event */
            int dbytes = (type == 0xC0 || type == 0xD0) ? 1 : 2;
            if (i + 1 + dbytes > n) goto out;
            uint8_t d0 = ev[i + 1];
            uint8_t d1 = dbytes == 2 ? ev[i + 2] : 0;
            size_t adv = 1 + dbytes;
            if (type == 0xC0) d0 = timb[d0 & 0x7F];      /* TIMB remap */
            uint32_t dur = 0;
            if (type == 0x90 && d1) {          /* XMI note-on duration */
                int lb = 0;
                size_t j = i + 3;
                do {
                    if (j >= n) goto out;
                    dur = (dur << 7) | (ev[j] & 0x7F);
                } while ((ev[j++] & 0x80) && ++lb < 4);
                adv = j - i;
            }
            FLUSH_UNTIL(now);
            if (bvar(trk, now - lastEmit) || bput(trk, &st, 1) ||
                bput(trk, &d0, 1)) goto out;
            if (dbytes == 2 && bput(trk, &d1, 1)) goto out;
            lastEmit = now;
            if (type == 0x90 && d1 && off_push(&offs, now + dur,
                                             (uint8_t)(st & 0x0F), d0))
                goto out;
            i += adv;
        }
    }
    FLUSH_UNTIL(~0u);
    rc = bput(trk, eot, 4);
out:
    off_free(&offs);
    return rc;
    #undef FLUSH_UNTIL
    #undef FLUSH_OFF
}

/* Walk FORM/CAT chunk content for FORM XMID children; seq picks one. */
static int find_evnt(const uint8_t* p, size_t n, int seq, int* idx,
                     const uint8_t** evnt, size_t* evntLen,
                     uint8_t timb[128]) {
    size_t i = 0;
    while (i + 8 <= n) {
        uint32_t sz = be32(p + i + 4);
        const uint8_t* d = p + i + 8;
        size_t avail = (i + 8 <= n) ? n - i - 8 : 0;
        if (sz > avail) sz = (uint32_t)avail;
        if (!memcmp(p + i, "FORM", 4) && sz >= 4) {
            if (!memcmp(d, "XMID", 4)) {
                if ((*idx)++ == seq) {
                    /* inside this FORM XMID find EVNT (+ TIMB first pass) */
                    size_t j = 4;
                    const uint8_t* ev = NULL; size_t evn = 0;
                    while (j + 8 <= sz) {
                        uint32_t csz = be32(d + j + 4);
                        if (j + 8 + csz > sz) csz = (uint32_t)(sz - j - 8);
                        if (!memcmp(d + j, "TIMB", 4)) {
                            for (size_t k = 0; k + 3 < csz; k += 4) {
                                uint16_t prog = d[j + 8 + k] | (d[j + 8 + k + 1] << 8);
                                uint16_t ptch = d[j + 8 + k + 2] | (d[j + 8 + k + 3] << 8);
                                if (prog < 128) timb[prog] = (uint8_t)ptch;
                            }
                        } else if (!memcmp(d + j, "EVNT", 4)) {
                            ev = d + j + 8; evn = csz;
                        }
                        j += 8 + csz + (csz & 1);
                    }
                    if (!ev) return -1;
                    *evnt = ev; *evntLen = evn;
                    return 0;
                }
            } else if (!memcmp(d, "XDIR", 4)) {
                /* catalog — keep walking children inside */
                if (find_evnt(d + 4, sz - 4, seq, idx, evnt, evntLen, timb) == 0)
                    return 0;
            } else {
                /* other FORM type — descend anyway (safety) */
                if (find_evnt(d + 4, sz - 4, seq, idx, evnt, evntLen, timb) == 0)
                    return 0;
            }
        } else if (!memcmp(p + i, "CAT ", 4) && sz >= 4) {
            if (find_evnt(d + 4, sz - 4, seq, idx, evnt, evntLen, timb) == 0)
                return 0;
        }
        i += 8 + sz + (sz & 1);
    }
    return -1;
}

/* Public: in=XMIDI buffer, seq=which sequence inside CAT (0-based).
   Returns malloc'd SMF via *out, length via *outLen. NULL on failure. */
int xmi_to_smf(const void* in, size_t inLen, int seq,
               uint8_t** out, size_t* outLen) {
    const uint8_t* p = (const uint8_t*)in;
    *out = NULL; *outLen = 0;
    if (!p || inLen < 12 || memcmp(p, "FORM", 4)) return -1;

    const uint8_t* ev; size_t evn;
    uint8_t timb[128];
    for (int i = 0; i < 128; ++i) timb[i] = (uint8_t)i;
    int idx = 0;
    if (find_evnt(p + 8 + 4, inLen - 12, seq, &idx, &ev, &evn, timb))
        return -1;

    Buf trk = {0};
    if (conv_evnt(ev, evn, timb, &trk)) { free(trk.p); return -1; }

    Buf out_ = {0};
    static const uint8_t mthd[14] = {
        'M','T','h','d', 0,0,0,6, 0,0, 0,1, 0,120       /* fmt0 1trk div120 */
    };
    if (bput(&out_, mthd, 14)) { free(trk.p); free(out_.p); return -1; }
    uint8_t mtrk[8] = {'M','T','r','k'};
    mtrk[4] = (uint8_t)(trk.n >> 24); mtrk[5] = (uint8_t)(trk.n >> 16);
    mtrk[6] = (uint8_t)(trk.n >> 8);  mtrk[7] = (uint8_t)trk.n;
    if (bput(&out_, mtrk, 8) || bput(&out_, trk.p, trk.n)) {
        free(trk.p); free(out_.p); return -1;
    }
    free(trk.p);
    *out = out_.p; *outLen = out_.n;
    return 0;
}
