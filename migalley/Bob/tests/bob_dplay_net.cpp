// bob_dplay_net — loopback test for the native DirectPlay4 implementation.
// Creates a host and a guest CDPlayNet in one process, talks over
// 127.0.0.1 (UDP discovery + TCP session), and exercises the surface
// bob/comms/comms.cpp uses: EnumConnections, InitializeConnection,
// EnumSessions, Open CREATE/JOIN, CreatePlayer/CreateGroup,
// AddPlayerToGroup, Send/SendEx routing (player/group/broadcast),
// Receive filtering, system messages, DestroyPlayer, Close.

#include "dplay_net.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

static int fails = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); ++fails; } \
    else         { printf("ok:   %s\n", msg); } } while (0)

static int connCalls = 0;
static BOOL WINAPI connCb(LPCGUID g, LPVOID conn, DWORD csz,
                          LPVOID name, DWORD flags, LPVOID) {
    ++connCalls;
    CHECK(g && g->Data1 == 0x36E95EE0, "EnumConnections reports TCPIP guid");
    RDPNAME* nm = (RDPNAME*)name;
    CHECK(nm && nm->shortName.a && strstr(nm->shortName.a, "TCP/IP"),
          "service name mentions TCP/IP");
    CHECK(conn && csz >= 8 && !memcmp(conn, "RDPNTCP", 7),
          "connection blob present");
    return TRUE;
}

struct SessInfo { GUID inst; std::string name; uint32_t curP, maxP; };
static std::vector<SessInfo> found;
static BOOL WINAPI sessCb(LPVOID d, LPDWORD, DWORD, LPVOID) {
    RDPSD2* s = (RDPSD2*)d;
    if (s && s->name.a) {
        SessInfo si; si.inst = s->guidInstance; si.name = s->name.a;
        si.curP = s->dwCurrentPlayers; si.maxP = s->dwMaxPlayers;
        found.push_back(si);
    }
    return TRUE;
}

static bool recv_for(CDPlayNet* o, DPXID me, std::vector<uint8_t>& out,
                     DPXID* fromOut, int tries = 40) {
    /* poll — network delivery is async */
    for (int i = 0; i < tries; ++i) {
        DPXID from = 0, to = me;
        uint8_t buf[4096]; uint32_t len = sizeof(buf);
        HRESULT r = o->Receive(&from, &to, 2 /*DPRECEIVE_TOPLAYER*/,
                               buf, &len);
        if (r == 0) {
            if (from == 0) continue;   /* skip DPSYS_* system messages */
            out.assign(buf, buf + len);
            if (fromOut) *fromOut = from;
            return true;
        }
        usleep(25000);
    }
    return false;
}

int main() {
    setenv("ROWAN_DPLAY_PORT", "54910", 1);

    CDPlayNet* host  = (CDPlayNet*)RowanDPlay_CreateInstance();
    CDPlayNet* guest = (CDPlayNet*)RowanDPlay_CreateInstance();

    /* --- service enumeration --- */
    connCalls = 0;
    CHECK(host->EnumConnections(nullptr, (LPVOID)&connCb, nullptr, 0) == 0,
          "EnumConnections DP_OK");
    CHECK(connCalls == 1, "exactly one service provider");

    RDPSD2 desc; memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidApplication.Data1 = 0xB0BDA7A;   /* arbitrary app guid */
    desc.dwMaxPlayers = 8;
    char sessName[] = "Loopback Skirmish";
    desc.name.a = sessName;

    /* --- host creates session --- */
    CHECK(host->Open(&desc, 2 /*DPOPEN_CREATE*/) == 0, "host Open(CREATE)");
    usleep(100000);

    /* --- guest discovers + joins --- */
    CHECK(guest->EnumConnections(nullptr, (LPVOID)&connCb, nullptr, 0) == 0,
          "guest EnumConnections");
    CHECK(guest->InitializeConnection((LPVOID)"RDPNTCP", 0) == 0,
          "guest InitializeConnection");

    /* EnumSessions is nonblocking (UI-timer safe): the query goes out on
       the call and the host's UDP reply lands on a *later* call's drain —
       so poll like the real frontend does. */
    found.clear();
    for (int i = 0; i < 40 && found.empty(); ++i) {
        CHECK(guest->EnumSessions(&desc, 0, (LPVOID)&sessCb, nullptr,
                                  0x10 /*ASYNC*/) == 0,
              "guest EnumSessions DP_OK");
        usleep(50000);
    }
    CHECK(!found.empty() && found[0].name == "Loopback Skirmish",
          "guest discovered host session over loopback");

    if (found.empty()) { printf("cannot continue\n"); return 1; }
    RDPSD2 jd; memset(&jd, 0, sizeof(jd));
    jd.dwSize = sizeof(jd);
    jd.guidInstance = found[0].inst;
    jd.guidApplication = desc.guidApplication;
    char jn[] = "Loopback Skirmish";
    jd.name.a = jn;
    CHECK(guest->Open(&jd, 1 /*DPOPEN_JOIN*/) == 0, "guest Open(JOIN)");

    /* --- players --- */
    DPXID agg = 0, me = 0, grp = 0, gme = 0;
    CHECK(host->CreatePlayer(&agg, nullptr, nullptr, nullptr, 0, 0) == 0,
          "host CreatePlayer(agg)");
    CHECK(host->CreateGroup(&grp, nullptr, nullptr, 0, 0) == 0,
          "host CreateGroup");
    CHECK(host->CreatePlayer(&me, nullptr, nullptr, nullptr, 0, 0) == 0,
          "host CreatePlayer(me)");
    CHECK(guest->CreatePlayer(&gme, nullptr, nullptr, nullptr, 0, 0) == 0,
          "guest CreatePlayer over wire");
    CHECK(gme != 0 && gme != me && gme != agg, "guest got distinct DPID");

    CHECK(guest->AddPlayerToGroup(grp, gme) == 0, "guest joins group");
    CHECK(host->AddPlayerToGroup(grp, me) == 0, "host joins group");
    usleep(100000);   /* let W_ADDG reach host */

    /* --- direct player->player message --- */
    const char* hello = "password pack";
    CHECK(guest->Send(gme, me, 0, (void*)hello, (uint32_t)strlen(hello) + 1) == 0,
          "guest Send to host player");
    std::vector<uint8_t> got; DPXID from = 0;
    CHECK(recv_for(host, me, got, &from), "host received direct message");
    if (!got.empty()) {
        CHECK(!strcmp((char*)got.data(), hello), "payload intact");
        CHECK(from == gme, "from field preserved");
    }

    /* --- broadcast --- */
    const char* bc = "prefs to all";
    CHECK(host->Send(me, 0 /*ALLPLAYERS*/, 0, (void*)bc, (uint32_t)strlen(bc) + 1) == 0,
          "host broadcast");
    got.clear();
    CHECK(recv_for(guest, gme, got, &from), "guest received broadcast");
    if (!got.empty()) CHECK(!strcmp((char*)got.data(), bc), "broadcast payload");

    /* --- group-addressed --- */
    const char* gm = "group ping";
    CHECK(host->Send(me, grp, 0, (void*)gm, (uint32_t)strlen(gm) + 1) == 0,
          "host send to group");
    got.clear();
    CHECK(recv_for(guest, gme, got, nullptr), "guest received group message");

    /* --- client -> broadcast reaches other direction --- */
    const char* cb = "chat back";
    CHECK(guest->Send(gme, 0, 0, (void*)cb, (uint32_t)strlen(cb) + 1) == 0,
          "guest broadcast");
    got.clear();
    CHECK(recv_for(host, me, got, nullptr), "host received client broadcast");

    /* --- system message on player destroy --- */
    CHECK(guest->DestroyPlayer(gme) == 0, "guest DestroyPlayer");
    bool sawDestroy = false;
    for (int i = 0; i < 40 && !sawDestroy; ++i) {
        DPXID f = 0, t = me; uint8_t buf[512]; uint32_t len = sizeof(buf);
        if (host->Receive(&f, &t, 0, buf, &len) == 0 && len >= 8) {
            uint32_t ty = *(uint32_t*)buf;
            uint32_t id = *(uint32_t*)(buf + 8);
            if (f == 0 && ty == 5 /*DPSYS_DESTROYPLAYERORGROUP*/ && id == gme)
                sawDestroy = true;
        }
        usleep(25000);
    }
    CHECK(sawDestroy, "host saw DPSYS_DESTROYPLAYERORGROUP for guest");

    /* --- teardown: guest leaves, then host closes --- */
    CHECK(guest->Close() == 0, "guest Close");
    usleep(200000);
    CHECK(host->Close() == 0, "host Close");
    guest->Release();
    host->Release();

    printf(fails ? "\n%d FAILURES\n" : "\nALL PASS\n", fails);
    return fails ? 1 : 0;
}
