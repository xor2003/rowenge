#include "dplay_net.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* ------------------------------------------------------------------
   HRESULT codes (real dplay.h values: MAKE_DPHRESULT = 0x88770000|code)
   ------------------------------------------------------------------ */
static const HRESULT DPX_OK               = 0;
static const HRESULT DPX_ERR_GENERIC      = (HRESULT)0x80004005L;
static const HRESULT DPX_ERR_NOINTERFACE  = (HRESULT)0x80004002L;
static const HRESULT DPX_ERR_INVALIDPARAM = (HRESULT)0x80070057L;
static const HRESULT DPX_ERR_UNSUPPORTED  = (HRESULT)0x80004021L;
static const HRESULT DPX_BUFFERTOOSMALL   = (HRESULT)0x8877001EL; /* 30 */
static const HRESULT DPX_CANTCREATEPLAYER = (HRESULT)0x8877003CL; /* 60 */
static const HRESULT DPX_CANTCREATEGROUP  = (HRESULT)0x88770032L; /* 50 */
static const HRESULT DPX_CANTCREATESESSION= (HRESULT)0x88770046L; /* 70 */
static const HRESULT DPX_INVALIDFLAGS     = (HRESULT)0x88770078L; /* 120 */
static const HRESULT DPX_INVALIDPLAYER    = (HRESULT)0x88770096L; /* 150 */
static const HRESULT DPX_INVALIDGROUP     = (HRESULT)0x8877009BL; /* 155 */
static const HRESULT DPX_NOCONNECTION     = (HRESULT)0x887700AAL; /* 170 */
static const HRESULT DPX_NOMESSAGES       = (HRESULT)0x887700BEL; /* 190 */

/* real dplay.h flag values (bob/h/DPLAY.H) */
enum {
    DPX_OPEN_JOIN           = 0x00000001,
    DPX_OPEN_CREATE         = 0x00000002,
    DPX_RECV_FROMPLAYER     = 0x00000004,
    DPX_RECV_TOPLAYER       = 0x00000002,
    DPX_RECV_PEEK           = 0x00000008,
    DPXID_ALLPLAYERS        = 0,
    DPXID_SYSMSG            = 0,
    DPXSYS_CREATEPLAYERORGROUP  = 0x0003,
    DPXSYS_DESTROYPLAYERORGROUP = 0x0005,
    DPXSYS_SESSIONLOST          = 0x0031,
    DPXPLAYERTYPE_GROUP         = 0,
    DPXPLAYERTYPE_PLAYER        = 1,
};

/* DPMSG_* system payloads (real layout — game casts them) */
#pragma pack(push, 1)
struct RMsgCreateP {
    uint32_t dwType, dwPlayerType, dpId, dwCurrentPlayers;
    void* lpData; uint32_t dwDataSize;
    RDPNAME  name;
    uint32_t dpIdParent, dwFlags;
};
struct RMsgDestroyP {
    uint32_t dwType, dwPlayerType, dpId;
    void* lpLocalData;  uint32_t dwLocalDataSize;
    void* lpRemoteData; uint32_t dwRemoteDataSize;
    RDPNAME  name;
    uint32_t dpIdParent, dwFlags;
};
struct RMsgAddPlayerToGroup { uint32_t dwType, dpIdGroup, dpIdPlayer; };
struct RMsgGeneric          { uint32_t dwType; };
struct RDPCAPS {                   /* real DPCAPS */
    uint32_t dwSize, dwFlags, dwMaxBufferSize, dwMaxQueueSize, dwMaxPlayers,
             dwHundredBaud, dwLatency, dwMaxLocalPlayers, dwHeaderLength, dwTimeout;
};
#pragma pack(pop)

/* ------------------------------------------------------------------
   Wire protocol (all integers little-endian; i386/x86_64 hosts both LE)
   ------------------------------------------------------------------ */
enum {
    W_MAGIC      = 0x4E504452,   /* 'RDPN' */
    WQ_MAGIC     = 0x31504452,   /* query  */
    WR_MAGIC     = 0x32504452,   /* reply  */
    W_HELLO      = 1,            /* c->h: app guid */
    W_MSG        = 2,            /* app message {from,to,payload} */
    W_CPREQ      = 3,            /* c->h: create {aux=reqTag,to=kind(1 player,2 group)} */
    W_CPACK      = 4,            /* h->c: assigned {aux=reqTag,to=dpid} */
    W_ADDG       = 5,            /* {to=group, from=player} */
    W_DELG       = 6,
    W_SYSMSG     = 7,            /* h->c: payload = DPMSG_* blob */
    W_DESTROY    = 8,            /* c->h: {from=dpid} */
    W_SESSLOST   = 9,            /* h->c */
    W_BYE        = 10,           /* c->h graceful */
};
struct WHdr {
    uint32_t magic, type, from, to, aux, len;
};

#pragma pack(push, 1)
struct UDQuery { uint32_t magic; GUID app; };
struct UDReply {
    uint32_t magic; GUID inst; GUID app;
    uint32_t flags, maxP, curP; char name[64];
};
#pragma pack(pop)

static bool dpn_log_enabled() {
    static int e = -1;
    if (e < 0) e = getenv("ROWAN_DPLAY_DEBUG") ? 1 : 0;
    return e;
}
#define DPNLOG(...) do { if (dpn_log_enabled()) { fprintf(stderr, "[dplay] " __VA_ARGS__); fflush(stderr); } } while (0)

static unsigned short dpn_udp_port() {
    const char* e = getenv("ROWAN_DPLAY_PORT");
    return (unsigned short)(e ? atoi(e) : 54710);
}
/* session traffic now runs over ENet (reliable ordered UDP) on udp+1;
   kept the old TCP port number so ROWAN_DPLAY_PORT stays compatible */
static unsigned short dpn_enet_port() { return (unsigned short)(dpn_udp_port() + 1); }

static void enetInitOnce() {
    static std::once_flag once;
    std::call_once(once, []{ enet_initialize(); });
}

/* ================================================================== */

CDPlayNet::CDPlayNet() {
    memset(&sessionDesc, 0, sizeof(sessionDesc));
    DPNLOG("created\n");
}

CDPlayNet::~CDPlayNet() {
    teardown();
    DPNLOG("destroyed\n");
}

HRESULT CDPlayNet::QueryInterface(REFIID, LPVOID* ppv) {
    if (!ppv) return DPX_ERR_INVALIDPARAM;
    *ppv = this;
    ++refs;
    return DPX_OK;
}
ULONG CDPlayNet::AddRef()  { return ++refs; }
ULONG CDPlayNet::Release() {
    ULONG r = --refs;
    if (!r) delete this;
    return r;
}

/* ---------------- teardown ---------------- */
void CDPlayNet::teardown() {
    shutting = true;
    /* ask the service thread to flush outQ, run the ENet disconnect
       handshake on every live peer, then exit — then destroy the host. */
    {
        std::lock_guard<std::mutex> lk(outMtx);
        draining = true;
    }
    if (serviceThr.joinable()) serviceThr.join();
    if (eHost) { enet_host_destroy(eHost); eHost = nullptr; }
    if (udpThr.joinable()) udpThr.join();
    {
        std::lock_guard<std::mutex> lk(mtx);
        conns.clear();
        hostConn.reset();
    }
    if (udpSrvFd >= 0) { close(udpSrvFd); udpSrvFd = -1; }
    if (udpQFd >= 0) { close(udpQFd); udpQFd = -1; }
    inSession = false;
    isHost = false;
    /* leave the object reusable for a later Open */
    draining = false;
    shutting = false;
    connectDone = false;
    connectFailed = false;
}

/* ---------------- queue ---------------- */
void CDPlayNet::enqueue(DPXID from, DPXID to, const void* data, size_t len) {
    Msg m;
    m.from = from; m.to = to;
    const uint8_t* p = (const uint8_t*)data;
    m.data.assign(p, p + len);
    queue.push_back(std::move(m));
    /* signal waiting players (aggregator event etc.) */
    for (auto& kv : localPlayers)
        if (kv.second && (to == DPXID_ALLPLAYERS || to == kv.first))
            SetEvent(kv.second);
}

void CDPlayNet::enqueueSysMsg(const void* blob, size_t len) {
    /* real DPlay queues a copy of each system message on EVERY player.
       One shared to=ALLPLAYERS copy gets stolen by whichever Receive runs
       first (e.g. the aggregator player draining its queue would eat a
       DESTROYPLAYER meant for the game loop) — so enqueue per player. */
    for (auto& kv : localPlayers)
        enqueue(DPXID_SYSMSG, kv.first, blob, len);
}

/* sysmsg blobs (real DPMSG layouts) */
void CDPlayNet::pushSysCreate(DPXID id, uint32_t ptype) {
    RMsgCreateP m; memset(&m, 0, sizeof(m));
    m.dwType = DPXSYS_CREATEPLAYERORGROUP;
    m.dwPlayerType = ptype;
    m.dpId = id;
    m.name.dwSize = sizeof(RDPNAME);
    enqueueSysMsg(&m, sizeof(m));
}
void CDPlayNet::pushSysDestroy(DPXID id, uint32_t ptype) {
    RMsgDestroyP m; memset(&m, 0, sizeof(m));
    m.dwType = DPXSYS_DESTROYPLAYERORGROUP;
    m.dwPlayerType = ptype;
    m.dpId = id;
    m.name.dwSize = sizeof(RDPNAME);
    enqueueSysMsg(&m, sizeof(m));
}

/* ---------------- wire helpers ---------------- */
/* Frame one WHdr message into a single reliable ENet packet and queue it
   for the service thread. Channel 0 is reliable+ordered — same ordering
   guarantee the TCP stream provided. */
void CDPlayNet::queuePacket(ENetPeer* peer, uint32_t type, DPXID from, DPXID to,
                            uint32_t aux, const void* data, size_t len) {
    if (!peer) return;
    OutPkt p;
    p.peer = peer;
    p.buf.resize(sizeof(WHdr) + len);
    WHdr* h = (WHdr*)p.buf.data();
    memset(h, 0, sizeof(*h));
    h->magic = W_MAGIC; h->type = type; h->from = from; h->to = to;
    h->aux = aux; h->len = (uint32_t)len;
    if (len) memcpy(p.buf.data() + sizeof(WHdr), data, len);
    std::lock_guard<std::mutex> lk(outMtx);
    outQ.push_back(std::move(p));
}

HRESULT CDPlayNet::sendMsgToWire(Conn* c, DPXID from, DPXID to, const void* data, size_t len) {
    if (!c || !c->peer || c->peer->state != ENET_PEER_STATE_CONNECTED)
        return DPX_ERR_GENERIC;
    queuePacket(c->peer, W_MSG, from, to, 0, data, len);
    return DPX_OK;
}

/* routing of app messages when WE are the host */
HRESULT CDPlayNet::hostRoute(Conn* src, DPXID from, DPXID to, const uint8_t* data, size_t len) {
    std::vector<std::pair<Conn*, DPXID> > targets;   /* conn, rewritten 'to' */
    bool local = false;
    {
        if (to == DPXID_ALLPLAYERS) {
            /* broadcast: every player gets a copy addressed to it -
               EXCEPT the sender. The game counts on no self-echo
               (resync 'needed' masks out the sender's own bit, so an
               echo of its own PID_RESYNC would wedge the count). */
            for (auto& c : conns)
                for (DPXID p : c->players)
                    if (p != from)
                        sendMsgToWire(c.get(), from, p, data, len);
            for (auto& kv : localPlayers)
                if (kv.first != from)
                    enqueue(from, kv.first, data, len);
            local = false;
        } else if (groups.count(to)) {              /* group address */
            /* every conn owning a member gets one copy (to=that member);
               the sender player never gets its own group message back */
            for (auto& c : conns) {
                for (DPXID p : groups[to])
                    if (p != from && connOwnsPlayer(c.get(), p)) { targets.push_back({c.get(), p}); break; }
            }
            for (DPXID p : groups[to])
                if (p != from && localPlayers.count(p))
                    enqueue(from, p, data, len);
            local = false;   /* already enqueued per-member */
        } else if (localPlayers.count(to)) {
            local = true;
        } else {
            for (auto& c : conns)
                if (connOwnsPlayer(c.get(), to)) { targets.push_back({c.get(), to}); break; }
        }
    }
    DPNLOG("route from=%u to=%u len=%zu local=%d targets=%zu pid=%08x\n",
           from, to, len, (int)local, targets.size(),
           len >= 4 ? *(const uint32_t*)data : 0);
    if (local) enqueue(from, to, data, len);
    for (auto& t : targets) sendMsgToWire(t.first, from, t.second, data, len);
    return DPX_OK;
}

/* send path for this object's local players */
HRESULT CDPlayNet::routeFromLocal(DPXID from, DPXID to, const void* data, size_t len) {
    if (isHost) {
        return hostRoute(nullptr, from, to, (const uint8_t*)data, len);
    }
    if (hostConn && hostConn->peer &&
        hostConn->peer->state == ENET_PEER_STATE_CONNECTED)
        return sendMsgToWire(hostConn.get(), from, to, data, len);
    /* no connection — loop back locally so single-host test builds still
       flow. mtx is already held by the caller (SendEx). */
    enqueue(from, to, data, len);
    return DPX_OK;
}

/* ---------------- host side ---------------- */
void CDPlayNet::hostDestroyRemotePlayers(Conn* c) {
    /* remove all of conn's players/groups, broadcast DESTROY sysmsgs */
    DPNLOG("conn teardown: %zu remote players\n", c->players.size());
    std::vector<DPXID> gone(c->players.begin(), c->players.end());
    for (DPXID p : gone) {
        for (auto& g : groups) g.second.erase(p);
        pushSysDestroy(p, DPXPLAYERTYPE_PLAYER);
        /* forward destroy to other clients */
        RMsgDestroyP m; memset(&m, 0, sizeof(m));
        m.dwType = DPXSYS_DESTROYPLAYERORGROUP;
        m.dwPlayerType = DPXPLAYERTYPE_PLAYER;
        m.dpId = p; m.name.dwSize = sizeof(RDPNAME);
        for (auto& o : conns)
            if (o.get() != c)
                queuePacket(o->peer, W_SYSMSG, 0, 0, 0, &m, sizeof(m));
    }
    c->players.clear();
}

/* client->host message dispatch (one ENet packet = one WHdr message) */
void CDPlayNet::hostOnPacket(Conn* c, const uint8_t* p, size_t n) {
    if (n < sizeof(WHdr)) return;
    WHdr h; memcpy(&h, p, sizeof(h));
    if (h.magic != W_MAGIC || sizeof(WHdr) + h.len > n) return;
    const uint8_t* pl = p + sizeof(WHdr);
    std::unique_lock<std::mutex> lk(mtx);
    switch (h.type) {
    case W_HELLO:
        DPNLOG("client hello\n");
        break;
    case W_MSG:
        hostRoute(c, h.from, h.to, pl, h.len);
        break;
    case W_CPREQ: {
        DPXID id = nextId++;
        c->players.insert(id);
        if (h.to == 1) {        /* player */
            pushSysCreate(id, DPXPLAYERTYPE_PLAYER);
            sessionDesc.dwCurrentPlayers++;
        } else {
            localGroups.insert(id);
            pushSysCreate(id, DPXPLAYERTYPE_GROUP);
        }
        queuePacket(c->peer, W_CPACK, 0, id, h.aux, nullptr, 0);
        /* tell other clients about the new player/group */
        RMsgCreateP m; memset(&m, 0, sizeof(m));
        m.dwType = DPXSYS_CREATEPLAYERORGROUP;
        m.dwPlayerType = h.to; m.dpId = id;
        m.dwCurrentPlayers = sessionDesc.dwCurrentPlayers;
        m.name.dwSize = sizeof(RDPNAME);
        for (auto& o : conns)
            if (o.get() != c)
                queuePacket(o->peer, W_SYSMSG, 0, 0, 0, &m, sizeof(m));
        break;
    }
    case W_ADDG:
        groups[h.to].insert(h.from);
        DPNLOG("wire addplayer g=%u p=%u members=%zu\n", h.to, h.from, groups[h.to].size());
        break;
    case W_DELG:
        groups[h.to].erase(h.from);
        DPNLOG("wire delplayer g=%u p=%u\n", h.to, h.from);
        break;
    case W_DESTROY: {
        DPXID pid = h.from;
        c->players.erase(pid);
        for (auto& g : groups) g.second.erase(pid);
        pushSysDestroy(pid, DPXPLAYERTYPE_PLAYER);
        RMsgDestroyP m; memset(&m, 0, sizeof(m));
        m.dwType = DPXSYS_DESTROYPLAYERORGROUP;
        m.dwPlayerType = DPXPLAYERTYPE_PLAYER;
        m.dpId = pid; m.name.dwSize = sizeof(RDPNAME);
        for (auto& o : conns)
            if (o.get() != c)
                queuePacket(o->peer, W_SYSMSG, 0, 0, 0, &m, sizeof(m));
        break;
    }
    case W_BYE:
        hostDestroyRemotePlayers(c);
        enet_peer_disconnect(c->peer, 0);     /* service thread — safe */
        break;
    default: break;
    }
}

/* UDP discovery responder (host) */
void CDPlayNet::udpResponderLoop() {
    while (!shutting) {
        fd_set rf; FD_ZERO(&rf); FD_SET(udpSrvFd, &rf);
        timeval tv{0, 200000};
        if (select(udpSrvFd + 1, &rf, nullptr, nullptr, &tv) <= 0) continue;
        UDQuery q; sockaddr_in sa; socklen_t sl = sizeof(sa);
        ssize_t n = recvfrom(udpSrvFd, &q, sizeof(q), 0, (sockaddr*)&sa, &sl);
        if (n != (ssize_t)sizeof(q) || q.magic != WQ_MAGIC) continue;
        {
            std::lock_guard<std::mutex> lk(mtx);
            if (!inSession || !isHost) continue;
            if (q.app != sessionDesc.guidApplication) continue;
        }
        UDReply r; memset(&r, 0, sizeof(r));
        r.magic = WR_MAGIC;
        {
            std::lock_guard<std::mutex> lk(mtx);
            r.inst = sessionDesc.guidInstance;
            r.app = sessionDesc.guidApplication;
            r.flags = sessionDesc.dwFlags;
            r.maxP = sessionDesc.dwMaxPlayers;
            r.curP = sessionDesc.dwCurrentPlayers;
            strncpy(r.name, sessionName.c_str(), sizeof(r.name) - 1);
        }
        sendto(udpSrvFd, &r, sizeof(r), 0, (sockaddr*)&sa, sl);
    }
}

/* ---------------- client side ---------------- */
/* host->client message dispatch */
void CDPlayNet::clientOnPacket(const uint8_t* p, size_t n) {
    if (n < sizeof(WHdr)) return;
    WHdr h; memcpy(&h, p, sizeof(h));
    if (h.magic != W_MAGIC || sizeof(WHdr) + h.len > n) return;
    const uint8_t* pl = p + sizeof(WHdr);
    std::unique_lock<std::mutex> lk(mtx);
    switch (h.type) {
    case W_MSG:
        DPNLOG("recv from=%u to=%u len=%u pid=%08x\n",
               h.from, h.to, h.len, h.len >= 4 ? *(const uint32_t*)pl : 0);
        enqueue(h.from, h.to, pl, h.len);
        break;
    case W_SYSMSG:
        enqueueSysMsg(pl, h.len);
        break;
    case W_CPACK:
        pendingIds[h.aux] = {h.to, true};
        lk.unlock();
        idCv.notify_all();
        break;
    case W_SESSLOST: {
        RMsgGeneric m; m.dwType = DPXSYS_SESSIONLOST;
        enqueueSysMsg(&m, sizeof(m));
        break;
    }
    default: break;
    }
}

/* ---------------- ENet service thread ----------------
   Owns eHost entirely: drains outQ (the only path outbound packets take),
   then services host events. Senders never touch enet directly. */
void CDPlayNet::enetServiceLoop() {
    bool done = false;
    while (!done) {
        /* drain outbound queue — enet_peer_send only on this thread.
           The empty check and the draining check share one critical
           section so a packet pushed just before `draining` was set
           is still sent (Close's W_BYE/W_SESSLOST rely on it). */
        for (;;) {
            OutPkt p;
            bool has = false;
            {
                std::lock_guard<std::mutex> lk(outMtx);
                if (!outQ.empty()) {
                    p = std::move(outQ.front());
                    outQ.pop_front();
                    has = true;
                } else if (draining) {
                    done = true;
                }
            }
            if (!has) break;
            if (!p.peer) continue;
            ENetPacket* pk = enet_packet_create(p.buf.data(), p.buf.size(),
                                                ENET_PACKET_FLAG_RELIABLE);
            if (pk) enet_peer_send(p.peer, 0, pk);
        }
        if (done) break;
        ENetEvent ev;
        /* 5 ms cap on outbound latency; enet_host_service also flushes
           queued sends each iteration */
        if (enet_host_service(eHost, &ev, 5) > 0)
            handleEnetEvent(ev);
    }
    gracefulDisconnect();
}

/* teardown path on the service thread: disconnect every live peer and pump
   until the handshake completes (bounded — a dead peer must not hang exit) */
void CDPlayNet::gracefulDisconnect() {
    if (!eHost) return;
    for (size_t i = 0; i < eHost->peerCount; ++i) {
        ENetPeer* p = &eHost->peers[i];
        if (p->state == ENET_PEER_STATE_CONNECTED)
            enet_peer_disconnect(p, 0);
    }
    const auto deadline = std::chrono::steady_clock::now()
                          + std::chrono::milliseconds(400);
    ENetEvent ev;
    while (std::chrono::steady_clock::now() < deadline) {
        size_t live = 0;
        for (size_t i = 0; i < eHost->peerCount; ++i) {
            ENetPeerState s = eHost->peers[i].state;
            if (s != ENET_PEER_STATE_DISCONNECTED &&
                s != ENET_PEER_STATE_ZOMBIE)
                ++live;
        }
        if (!live) break;
        enet_host_service(eHost, &ev, 20);   /* events dropped — tearing down */
    }
    enet_host_flush(eHost);
}

void CDPlayNet::handleEnetEvent(const ENetEvent& ev) {
    switch (ev.type) {
    case ENET_EVENT_TYPE_CONNECT:
        if (isHost) {
            auto c = std::make_shared<Conn>();
            c->peer = ev.peer;
            c->addr.sin_addr.s_addr = ev.peer->address.host;  /* net order */
            c->addr.sin_port = htons(ev.peer->address.port);  /* host order */
            c->addr.sin_family = AF_INET;
            ev.peer->data = c.get();
            {
                std::lock_guard<std::mutex> lk(mtx);
                conns.push_back(c);
            }
            DPNLOG("client connected (enet)\n");
        } else {
            {
                std::lock_guard<std::mutex> lk(mtx);
                auto c = std::make_shared<Conn>();
                c->peer = ev.peer;
                ev.peer->data = c.get();
                hostConn = c;
                connectDone = true;
            }
            connCv.notify_all();
            DPNLOG("connected to host (enet)\n");
        }
        break;
    case ENET_EVENT_TYPE_RECEIVE:
        if (isHost)
            hostOnPacket((Conn*)ev.peer->data, ev.packet->data,
                         ev.packet->dataLength);
        else
            clientOnPacket(ev.packet->data, ev.packet->dataLength);
        enet_packet_destroy(ev.packet);
        break;
    case ENET_EVENT_TYPE_DISCONNECT:
        if (isHost) {
            Conn* c = (Conn*)ev.peer->data;
            ev.peer->data = nullptr;
            if (c) {
                std::lock_guard<std::mutex> lk(mtx);
                hostDestroyRemotePlayers(c);
                for (auto it = conns.begin(); it != conns.end(); ++it)
                    if (it->get() == c) { conns.erase(it); break; }
            }
            DPNLOG("client disconnected\n");
        } else {
            bool early;
            {
                std::lock_guard<std::mutex> lk(mtx);
                early = !connectDone;
                if (early) connectFailed = true;
                if (!early && inSession && !shutting) {
                    RMsgGeneric m; m.dwType = DPXSYS_SESSIONLOST;
                    enqueueSysMsg(&m, sizeof(m));
                }
                if (!early) inSession = false;
                if (hostConn) hostConn.reset();
            }
            if (early) connCv.notify_all();
            DPNLOG("host connection lost\n");
        }
        break;
    default: break;
    }
}

HRESULT CDPlayNet::requestId(DPXID* out, uint32_t kind) {
    uint32_t tag;
    ENetPeer* peer;
    {
        std::lock_guard<std::mutex> lk(mtx);
        tag = reqTagSeq++;
        if (!hostConn || !hostConn->peer ||
            hostConn->peer->state != ENET_PEER_STATE_CONNECTED)
            return DPX_NOCONNECTION;
        peer = hostConn->peer;
    }
    queuePacket(peer, W_CPREQ, 0, kind, tag, nullptr, 0);
    std::unique_lock<std::mutex> lk(mtx);
    bool ok = idCv.wait_for(lk, std::chrono::seconds(10),
                            [&]{ return pendingIds.count(tag) && pendingIds[tag].second; });
    if (!ok) return DPX_CANTCREATEPLAYER;
    *out = pendingIds[tag].first;
    pendingIds.erase(tag);
    return DPX_OK;
}

/* ===================== IDirectPlay2/3/4 surface ===================== */

HRESULT CDPlayNet::EnumConnections(LPCGUID appGuid, LPVOID cbv, LPVOID ctx, uint32_t) {
    if (!cbv) return DPX_ERR_INVALIDPARAM;
    EnumConnCb cb = (EnumConnCb)cbv;
    /* Our one provider: native TCP/IP. The connection blob is opaque to
       the game and echoed back through InitializeConnection. */
    static const GUID TCPIP = {0x36E95EE0,0x8577,0x11cf,{0x96,0x0C,0x00,0x80,0xC7,0x53,0x4E,0x82}};
    static const uint8_t blob[] = {'R','D','P','N','T','C','P','\0'};
    RDPNAME nm; memset(&nm, 0, sizeof(nm));
    nm.dwSize = sizeof(RDPNAME);
    char svcname[] = "Internet TCP/IP Connection For DirectPlay";
    nm.shortName.a = svcname;
    cb(&TCPIP, (LPVOID)blob, sizeof(blob), &nm, 0, ctx);
    (void)appGuid;
    return DPX_OK;
}

HRESULT CDPlayNet::InitializeConnection(LPVOID conn, uint32_t) {
    /* blob content is ours; presence of any blob selects TCP/IP */
    std::lock_guard<std::mutex> lk(mtx);
    initConn = true;
    (void)conn;
    return DPX_OK;
}

HRESULT CDPlayNet::EnumSessions(LPVOID descp, uint32_t, LPVOID cbv,
                                LPVOID ctx, uint32_t flags) {
    if (!cbv) return DPX_ERR_INVALIDPARAM;
    EnumSessCb cb = (EnumSessCb)cbv;
    RDPSD2* filt = (RDPSD2*)descp;

    if (udpQFd < 0) {
        udpQFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpQFd < 0) return DPX_ERR_GENERIC;
        int one = 1;
        setsockopt(udpQFd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
        setsockopt(udpQFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        /* non-blocking: the game re-enumerates on a 0-interval UI timer,
           so this must never block the message loop */
        fcntl(udpQFd, F_SETFL, fcntl(udpQFd, F_GETFL, 0) | O_NONBLOCK);
    }

    uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    /* rate-limited discovery query: LAN broadcast + loopback (+opt. direct) */
    if (nowMs - lastQueryMs >= 400) {
        lastQueryMs = nowMs;
        UDQuery q; q.magic = WQ_MAGIC;
        memset(&q.app, 0, sizeof(q.app));
        if (filt) q.app = filt->guidApplication;
        sockaddr_in dst{}; dst.sin_family = AF_INET;
        dst.sin_port = htons(dpn_udp_port());
        dst.sin_addr.s_addr = INADDR_BROADCAST;
        sendto(udpQFd, &q, sizeof(q), 0, (sockaddr*)&dst, sizeof(dst));
        dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sendto(udpQFd, &q, sizeof(q), 0, (sockaddr*)&dst, sizeof(dst));
        if (const char* ip = getenv("ROWAN_DPLAY_CONNECT")) {
            sockaddr_in d2{}; d2.sin_family = AF_INET;
            d2.sin_port = htons(dpn_udp_port());
            if (inet_aton(ip, &d2.sin_addr))
                sendto(udpQFd, &q, sizeof(q), 0, (sockaddr*)&d2, sizeof(d2));
        }
    }

    /* drain all pending replies into the host table */
    for (;;) {
        UDReply r; sockaddr_in sa; socklen_t sl = sizeof(sa);
        ssize_t n = recvfrom(udpQFd, &r, sizeof(r), 0, (sockaddr*)&sa, &sl);
        if (n != (ssize_t)sizeof(r)) break;
        if (r.magic != WR_MAGIC) continue;
        if (filt && filt->guidApplication.Data1 && r.app != filt->guidApplication)
            continue;
        std::string key((char*)&r.inst, sizeof(r.inst));
        DiscHost& h = discHosts[key];
        h.sa = sa; h.inst = r.inst; h.app = r.app;
        h.flags = r.flags; h.maxP = r.maxP; h.curP = r.curP;
        memcpy(h.name, r.name, sizeof(h.name));
        h.name[sizeof(h.name) - 1] = 0;
        h.lastSeenMs = nowMs;
    }

    /* expire dead hosts (they refresh on every query round) */
    for (auto it = discHosts.begin(); it != discHosts.end(); ) {
        if (nowMs - it->second.lastSeenMs > 4000)
            it = discHosts.erase(it);
        else
            ++it;
    }

    /* report all live sessions — the game rebuilds its list on every call */
    DWORD timeoutDummy = 0;
    for (auto& kv : discHosts) {
        const DiscHost& h = kv.second;
        RDPSD2 sd; memset(&sd, 0, sizeof(sd));
        sd.dwSize = sizeof(RDPSD2);
        sd.dwFlags = h.flags;
        sd.guidInstance = h.inst;
        sd.guidApplication = h.app;
        sd.dwMaxPlayers = h.maxP;
        sd.dwCurrentPlayers = h.curP;
        sd.name.a = (char*)h.name;
        if (!cb(&sd, &timeoutDummy, 0, ctx)) break;
    }
    (void)flags;
    return DPX_OK;
}

HRESULT CDPlayNet::Open(LPVOID descp, uint32_t flags) {
    RDPSD2* d = (RDPSD2*)descp;
    if (!d) return DPX_ERR_INVALIDPARAM;
    std::unique_lock<std::mutex> lk(mtx);
    if (inSession) return DPX_ERR_GENERIC;

    enetInitOnce();

    if (flags & DPX_OPEN_CREATE) {
        /* ENet server host — reliable ordered UDP session channel */
        ENetAddress ea;
        ea.host = ENET_HOST_ANY;
        ea.port = dpn_enet_port();
        eHost = enet_host_create(&ea, 32 /*peers*/, 1 /*channel*/, 0, 0);
        if (!eHost) return DPX_CANTCREATESESSION;
        /* UDP discovery responder (ENet has no broadcast discovery) */
        udpSrvFd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udpSrvFd >= 0) {
            int one = 1;
            setsockopt(udpSrvFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
            sockaddr_in ua{}; ua.sin_family = AF_INET;
            ua.sin_port = htons(dpn_udp_port());
            ua.sin_addr.s_addr = INADDR_ANY;
            if (bind(udpSrvFd, (sockaddr*)&ua, sizeof(ua))) {
                close(udpSrvFd); udpSrvFd = -1;
            }
        }
        sessionDesc = *d;
        if (!memcmp(&sessionDesc.guidInstance, "\0\0\0\0", 4)) {
            /* assign a fresh instance guid */
            sessionDesc.guidInstance.Data1 = (uint32_t)rand();
            sessionDesc.guidInstance.Data2 = (uint16_t)rand();
            sessionDesc.guidInstance.Data3 = (uint16_t)rand();
            for (auto& b : sessionDesc.guidInstance.Data4) b = (uint8_t)rand();
        }
        sessionName = d->name.a ? d->name.a : "BoB session";
        sessionDesc.dwCurrentPlayers = 0;
        isHost = true;
        inSession = true;
        serviceThr = std::thread(&CDPlayNet::enetServiceLoop, this);
        if (udpSrvFd >= 0)
            udpThr = std::thread(&CDPlayNet::udpResponderLoop, this);
        DPNLOG("hosting '%s' enet=%u udp=%u\n", sessionName.c_str(),
               dpn_enet_port(), dpn_udp_port());
        return DPX_OK;
    }

    if (flags & DPX_OPEN_JOIN) {
        /* locate host via discovery map or env override */
        sockaddr_in sa{}; bool have = false;
        std::string key((char*)&d->guidInstance, sizeof(d->guidInstance));
        auto it = discHosts.find(key);
        if (it != discHosts.end()) { sa = it->second.sa; have = true; }
        if (const char* ip = getenv("ROWAN_DPLAY_CONNECT")) {
            sockaddr_in d2{}; d2.sin_family = AF_INET;
            if (inet_aton(ip, &d2.sin_addr)) { sa = d2; have = true; }
        }
        if (!have) {
            DPNLOG("join failed: unknown session guid\n");
            return DPX_NOCONNECTION;
        }
        eHost = enet_host_create(nullptr /*client*/, 1, 1, 0, 0);
        if (!eHost) return DPX_NOCONNECTION;
        ENetAddress ea;
        ea.host = sa.sin_addr.s_addr;   /* enet stores host in NET order —
                                           copies it into sin_addr verbatim */
        ea.port = dpn_enet_port();      /* port IS host order in ENet */
        ENetPeer* peer = enet_host_connect(eHost, &ea, 1, 0);
        if (!peer) {
            enet_host_destroy(eHost); eHost = nullptr;
            return DPX_NOCONNECTION;
        }
        connectDone = false; connectFailed = false;
        isHost = false;
        serviceThr = std::thread(&CDPlayNet::enetServiceLoop, this);
        /* wait for the CONNECT event — timed wait, and NOT holding mtx
           while blocked (the service thread needs it to set hostConn) */
        bool ok = connCv.wait_for(lk, std::chrono::seconds(5),
                                  [&]{ return connectDone || connectFailed; });
        if (!ok || connectFailed || !hostConn) {
            DPNLOG("join failed: no connect from %s:%u\n",
                   inet_ntoa(sa.sin_addr), dpn_enet_port());
            lk.unlock();
            { std::lock_guard<std::mutex> ol(outMtx); draining = true; }
            serviceThr.join();
            enet_host_destroy(eHost); eHost = nullptr;
            draining = false;             /* allow a later Open retry */
            lk.lock();
            return DPX_NOCONNECTION;
        }
        sessionDesc = *d;
        sessionName = d->name.a ? d->name.a : "";
        inSession = true;
        queuePacket(hostConn->peer, W_HELLO, 0, 0, 0,
                    &d->guidApplication, sizeof(GUID));
        DPNLOG("joined session at %s\n", inet_ntoa(sa.sin_addr));
        return DPX_OK;
    }
    return DPX_INVALIDFLAGS;
}

HRESULT CDPlayNet::CreatePlayer(DPXID* out, LPVOID, HANDLE hEvent,
                                LPVOID, uint32_t, uint32_t) {
    if (!out) return DPX_ERR_INVALIDPARAM;
    std::unique_lock<std::mutex> lk(mtx);
    if (!inSession) return DPX_ERR_GENERIC;
    DPXID id;
    if (isHost) {
        id = nextId++;
        sessionDesc.dwCurrentPlayers++;
        lk.unlock();
        /* inform all clients */
        RMsgCreateP m; memset(&m, 0, sizeof(m));
        m.dwType = DPXSYS_CREATEPLAYERORGROUP;
        m.dwPlayerType = DPXPLAYERTYPE_PLAYER;
        m.dpId = id;
        m.dwCurrentPlayers = sessionDesc.dwCurrentPlayers;
        m.name.dwSize = sizeof(RDPNAME);
        std::vector<ENetPeer*> peers;
        {
            std::lock_guard<std::mutex> lk2(mtx);
            for (auto& c : conns) peers.push_back(c->peer);
        }
        for (ENetPeer* p : peers)
            queuePacket(p, W_SYSMSG, 0, 0, 0, &m, sizeof(m));
        lk.lock();
    } else {
        lk.unlock();
        HRESULT hr = requestId(&id, 1);
        if (hr != DPX_OK) return hr;
        lk.lock();
    }
    localPlayers[id] = hEvent;
    lk.unlock();
    {
        std::lock_guard<std::mutex> lk2(mtx);
        pushSysCreate(id, DPXPLAYERTYPE_PLAYER);
    }
    *out = id;
    DPNLOG("created player %u\n", id);
    return DPX_OK;
}

HRESULT CDPlayNet::CreateGroup(DPXID* out, LPVOID, LPVOID, uint32_t, uint32_t) {
    if (!out) return DPX_ERR_INVALIDPARAM;
    std::unique_lock<std::mutex> lk(mtx);
    if (!inSession) return DPX_ERR_GENERIC;
    DPXID id;
    if (isHost) {
        id = nextId++;
    } else {
        lk.unlock();
        HRESULT hr = requestId(&id, 2);
        if (hr != DPX_OK) return hr;
        lk.lock();
    }
    localGroups.insert(id);
    groups[id];  /* ensure membership set exists */
    lk.unlock();
    {
        std::lock_guard<std::mutex> lk2(mtx);
        pushSysCreate(id, DPXPLAYERTYPE_GROUP);
    }
    *out = id;
    DPNLOG("created group %u\n", id);
    return DPX_OK;
}

HRESULT CDPlayNet::AddPlayerToGroup(DPXID group, DPXID player) {
    std::unique_lock<std::mutex> lk(mtx);
    groups[group].insert(player);
    DPNLOG("addplayer g=%u p=%u members=%zu\n", group, player, groups[group].size());
    if (!isHost && hostConn && hostConn->peer)
        queuePacket(hostConn->peer, W_ADDG, player, group, 0, nullptr, 0);
    return DPX_OK;
}

HRESULT CDPlayNet::DeletePlayerFromGroup(DPXID group, DPXID player) {
    std::unique_lock<std::mutex> lk(mtx);
    auto it = groups.find(group);
    if (it != groups.end()) it->second.erase(player);
    DPNLOG("delplayer g=%u p=%u\n", group, player);
    if (!isHost && hostConn && hostConn->peer)
        queuePacket(hostConn->peer, W_DELG, player, group, 0, nullptr, 0);
    return DPX_OK;
}
HRESULT CDPlayNet::DestroyPlayer(DPXID id) {
    std::unique_lock<std::mutex> lk(mtx);
    localPlayers.erase(id);
    for (auto& g : groups) g.second.erase(id);
    if (!isHost && hostConn && hostConn->peer) {
        queuePacket(hostConn->peer, W_DESTROY, id, 0, 0, nullptr, 0);
        return DPX_OK;
    }
    pushSysDestroy(id, DPXPLAYERTYPE_PLAYER);
    return DPX_OK;
}

HRESULT CDPlayNet::DestroyGroup(DPXID id) {
    std::lock_guard<std::mutex> lk(mtx);
    groups.erase(id);
    localGroups.erase(id);
    pushSysDestroy(id, DPXPLAYERTYPE_GROUP);
    return DPX_OK;
}

HRESULT CDPlayNet::Send(DPXID from, DPXID to, uint32_t, LPVOID data, uint32_t len) {
    return SendEx(from, to, 0, data, len, 0, 0, nullptr, nullptr);
}

HRESULT CDPlayNet::SendEx(DPXID from, DPXID to, uint32_t, LPVOID data,
                          uint32_t len, uint32_t, uint32_t, LPVOID, uintptr_t*) {
    if (!data && len) return DPX_ERR_INVALIDPARAM;
    if (!inSession) return DPX_ERR_GENERIC;
    std::lock_guard<std::mutex> lk(mtx);
    return routeFromLocal(from, to, data, len);
}

HRESULT CDPlayNet::Receive(DPXID* pFrom, DPXID* pTo, uint32_t flags,
                           LPVOID buf, uint32_t* pLen) {
    if (!pFrom || !pTo || !pLen) return DPX_ERR_INVALIDPARAM;
    std::lock_guard<std::mutex> lk(mtx);
    DPXID wantFrom = pFrom ? *pFrom : 0;
    DPXID wantTo   = pTo ? *pTo : 0;
    for (auto it = queue.begin(); it != queue.end(); ++it) {
        bool ok = true;
        if (flags & DPX_RECV_FROMPLAYER)
            ok = (it->from == wantFrom);
        else if (flags & DPX_RECV_TOPLAYER)
            ok = (it->to == wantTo || it->to == DPXID_ALLPLAYERS);
        /* DPRECEIVE_ALL / 0 -> accept everything queued for us */
        if (!ok) continue;
        if (*pLen < it->data.size()) {
            *pLen = (uint32_t)it->data.size();
            return DPX_BUFFERTOOSMALL;
        }
        if (buf && !it->data.empty())
            memcpy(buf, it->data.data(), it->data.size());
        *pFrom = it->from;
        *pTo = it->to;
        *pLen = (uint32_t)it->data.size();
        DPNLOG("deq from=%u to=%u len=%u q=%zu pid=%08x\n",
               it->from, it->to, (uint32_t)it->data.size(), queue.size(),
               it->data.size() >= 4 ? *(const uint32_t*)it->data.data() : 0);
        if (!(flags & DPX_RECV_PEEK))
            queue.erase(it);
        return DPX_OK;
    }
    return DPX_NOMESSAGES;
}

HRESULT CDPlayNet::Close() {
    DPNLOG("Close\n");
    bool wasHost;
    std::shared_ptr<Conn> hc;
    {
        std::lock_guard<std::mutex> lk(mtx);
        wasHost = isHost;
        hc = hostConn;
    }
    if (wasHost) {
        /* notify clients before tearing down */
        std::vector<ENetPeer*> peers;
        {
            std::lock_guard<std::mutex> lk(mtx);
            for (auto& c : conns) peers.push_back(c->peer);
        }
        for (ENetPeer* p : peers)
            queuePacket(p, W_SESSLOST, 0, 0, 0, nullptr, 0);
    } else if (hc && hc->peer) {
        queuePacket(hc->peer, W_BYE, 0, 0, 0, nullptr, 0);
    }
    teardown();
    return DPX_OK;
}

HRESULT CDPlayNet::GetCaps(LPVOID p, uint32_t) {
    if (!p) return DPX_ERR_INVALIDPARAM;
    RDPCAPS c; memset(&c, 0, sizeof(c));
    c.dwSize = sizeof(RDPCAPS);
    c.dwMaxBufferSize = 65536;
    c.dwMaxPlayers = 256;
    c.dwMaxLocalPlayers = 256;
    c.dwHundredBaud = 100000;      /* ~10 Mbit nominal */
    c.dwLatency = 1;
    memcpy(p, &c, sizeof(c) < ((RDPCAPS*)p)->dwSize ? sizeof(c) : ((RDPCAPS*)p)->dwSize);
    return DPX_OK;
}

HRESULT CDPlayNet::GetMessageQueue(DPXID, DPXID, uint32_t,
                                   uint32_t* pNumMsgs, uint32_t* pNumBytes) {
    std::lock_guard<std::mutex> lk(mtx);
    if (pNumMsgs) *pNumMsgs = (uint32_t)queue.size();
    if (pNumBytes) {
        uint32_t n = 0;
        for (auto& m : queue) n += (uint32_t)m.data.size();
        *pNumBytes = n;
    }
    return DPX_OK;
}

HRESULT CDPlayNet::GetMessageCount(DPXID, uint32_t* p) {
    if (!p) return DPX_ERR_INVALIDPARAM;
    std::lock_guard<std::mutex> lk(mtx);
    *p = (uint32_t)queue.size();
    return DPX_OK;
}

HRESULT CDPlayNet::GetSessionDesc(LPVOID buf, uint32_t* pLen) {
    std::lock_guard<std::mutex> lk(mtx);
    uint32_t need = sizeof(RDPSD2);
    if (!pLen) return DPX_ERR_INVALIDPARAM;
    if (!buf || *pLen < need) { *pLen = need; return DPX_BUFFERTOOSMALL; }
    memcpy(buf, &sessionDesc, need);
    *pLen = need;
    return DPX_OK;
}
HRESULT CDPlayNet::SetSessionDesc(LPVOID d, uint32_t) {
    if (!d) return DPX_ERR_INVALIDPARAM;
    std::lock_guard<std::mutex> lk(mtx);
    sessionDesc = *(RDPSD2*)d;
    return DPX_OK;
}

HRESULT CDPlayNet::GetPlayerName(DPXID id, LPVOID buf, uint32_t* pLen) {
    std::lock_guard<std::mutex> lk(mtx);
    if (!localPlayers.count(id)) {
        /* still answer for remote ids: empty DPNAME */
    }
    uint32_t need = sizeof(RDPNAME);
    if (!pLen) return DPX_ERR_INVALIDPARAM;
    if (!buf || *pLen < need) { *pLen = need; return DPX_BUFFERTOOSMALL; }
    RDPNAME nm; memset(&nm, 0, sizeof(nm)); nm.dwSize = sizeof(RDPNAME);
    memcpy(buf, &nm, need); *pLen = need;
    return DPX_OK;
}

/* ---------- trivially-supported / unused-by-BoB surface ---------- */
HRESULT CDPlayNet::Initialize(LPGUID) { return DPX_OK; }
HRESULT CDPlayNet::CancelMessage(uint32_t, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::CancelPriority(uint32_t, uint32_t, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::EnumGroupPlayers(DPXID, LPGUID, LPVOID, LPVOID, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::EnumGroups(LPGUID, LPVOID, LPVOID, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::EnumPlayers(LPGUID, LPVOID, LPVOID, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::EnumGroupsInGroup(DPXID, LPGUID, LPVOID, LPVOID, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::AddGroupToGroup(DPXID, DPXID) { return DPX_OK; }
HRESULT CDPlayNet::CreateGroupInGroup(DPXID, DPXID*, LPVOID, LPVOID, uint32_t, uint32_t) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::DeleteGroupFromGroup(DPXID, DPXID) { return DPX_OK; }
HRESULT CDPlayNet::GetGroupConnectionSettings(uint32_t, DPXID, LPVOID, uint32_t*) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::SecureOpen(LPVOID d, uint32_t f, LPVOID, LPVOID) { return Open(d, f); }
HRESULT CDPlayNet::SendChatMessage(DPXID, DPXID, uint32_t, LPVOID) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::SetGroupConnectionSettings(uint32_t, DPXID, LPVOID) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::StartSession(uint32_t, DPXID) { return DPX_OK; }
HRESULT CDPlayNet::GetGroupFlags(DPXID, uint32_t* p) { if (p) *p = 0; return DPX_OK; }
HRESULT CDPlayNet::GetGroupParent(DPXID, DPXID*) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetPlayerAccount(DPXID, uint32_t, LPVOID, uint32_t*) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetPlayerFlags(DPXID, uint32_t* p) { if (p) *p = 0; return DPX_OK; }
HRESULT CDPlayNet::GetGroupOwner(DPXID, DPXID* p) { if (p) *p = 0; return DPX_OK; }
HRESULT CDPlayNet::SetGroupOwner(DPXID, DPXID) { return DPX_OK; }
HRESULT CDPlayNet::GetGroupData(DPXID, LPVOID, uint32_t*, uint32_t) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetGroupName(DPXID, LPVOID, uint32_t*) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetPlayerAddress(DPXID, LPVOID, uint32_t*) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetPlayerCaps(DPXID, LPVOID, uint32_t) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::GetPlayerData(DPXID, LPVOID, uint32_t*, uint32_t) { return DPX_ERR_UNSUPPORTED; }
HRESULT CDPlayNet::SetGroupData(DPXID, LPVOID, uint32_t, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::SetGroupName(DPXID, LPVOID, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::SetPlayerData(DPXID, LPVOID, uint32_t, uint32_t) { return DPX_OK; }
HRESULT CDPlayNet::SetPlayerName(DPXID, LPVOID, uint32_t) { return DPX_OK; }

/* ------------------------------------------------------------------ */
void* RowanDPlay_CreateInstance() {
    return new CDPlayNet();
}
