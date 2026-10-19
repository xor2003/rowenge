#pragma once

// Native port-to-port DirectPlay4 session implementation.
// Returns an object whose vtable matches the real IDirectPlay4A
// (IUnknown + IDirectPlay2 + IDirectPlay3 + IDirectPlay4 method order
// from bob/h/DPLAY.H). Created via CoCreateInstance in MFC_stub.cpp.

#include "WIN32_COMPAT.H"

#include <enet/enet.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <netinet/in.h>

typedef uint32_t DPXID;

/* Real-header struct layouts the game casts/fills directly. */
#pragma pack(push, 1)
struct RDPSD2 {                    /* real DPSESSIONDESC2 (64-bit ptr fields) */
    uint32_t dwSize;
    uint32_t dwFlags;
    GUID     guidInstance;
    GUID     guidApplication;
    uint32_t dwMaxPlayers;
    uint32_t dwCurrentPlayers;
    union { wchar_t* w; char* a; } name;
    union { wchar_t* w; char* a; } password;
    uintptr_t dwReserved1, dwReserved2, dwUser1, dwUser2, dwUser3, dwUser4;
};
struct RDPNAME {                   /* real DPNAME */
    uint32_t dwSize, dwFlags;
    union { wchar_t* w; char* a; } shortName;
    union { wchar_t* w; char* a; } longName;
};
#pragma pack(pop)

class CDPlayNet {
public:
    /* IUnknown */
    virtual HRESULT QueryInterface(REFIID, LPVOID*);
    virtual ULONG   AddRef();
    virtual ULONG   Release();
    /* IDirectPlay2 — declaration order in bob/h/DPLAY.H */
    virtual HRESULT AddPlayerToGroup(DPXID, DPXID);
    virtual HRESULT Close();
    virtual HRESULT CreateGroup(DPXID*, LPVOID, LPVOID, uint32_t, uint32_t);
    virtual HRESULT CreatePlayer(DPXID*, LPVOID, HANDLE, LPVOID, uint32_t, uint32_t);
    virtual HRESULT DeletePlayerFromGroup(DPXID, DPXID);
    virtual HRESULT DestroyGroup(DPXID);
    virtual HRESULT DestroyPlayer(DPXID);
    virtual HRESULT EnumGroupPlayers(DPXID, LPGUID, LPVOID, LPVOID, uint32_t);
    virtual HRESULT EnumGroups(LPGUID, LPVOID, LPVOID, uint32_t);
    virtual HRESULT EnumPlayers(LPGUID, LPVOID, LPVOID, uint32_t);
    virtual HRESULT EnumSessions(LPVOID, uint32_t, LPVOID, LPVOID, uint32_t);
    virtual HRESULT GetCaps(LPVOID, uint32_t);
    virtual HRESULT GetGroupData(DPXID, LPVOID, uint32_t*, uint32_t);
    virtual HRESULT GetGroupName(DPXID, LPVOID, uint32_t*);
    virtual HRESULT GetMessageCount(DPXID, uint32_t*);
    virtual HRESULT GetPlayerAddress(DPXID, LPVOID, uint32_t*);
    virtual HRESULT GetPlayerCaps(DPXID, LPVOID, uint32_t);
    virtual HRESULT GetPlayerData(DPXID, LPVOID, uint32_t*, uint32_t);
    virtual HRESULT GetPlayerName(DPXID, LPVOID, uint32_t*);
    virtual HRESULT GetSessionDesc(LPVOID, uint32_t*);
    virtual HRESULT Initialize(LPGUID);
    virtual HRESULT Open(LPVOID, uint32_t);
    virtual HRESULT Receive(DPXID*, DPXID*, uint32_t, LPVOID, uint32_t*);
    virtual HRESULT Send(DPXID, DPXID, uint32_t, LPVOID, uint32_t);
    virtual HRESULT SetGroupData(DPXID, LPVOID, uint32_t, uint32_t);
    virtual HRESULT SetGroupName(DPXID, LPVOID, uint32_t);
    virtual HRESULT SetPlayerData(DPXID, LPVOID, uint32_t, uint32_t);
    virtual HRESULT SetPlayerName(DPXID, LPVOID, uint32_t);
    virtual HRESULT SetSessionDesc(LPVOID, uint32_t);
    /* IDirectPlay3 */
    virtual HRESULT AddGroupToGroup(DPXID, DPXID);
    virtual HRESULT CreateGroupInGroup(DPXID, DPXID*, LPVOID, LPVOID, uint32_t, uint32_t);
    virtual HRESULT DeleteGroupFromGroup(DPXID, DPXID);
    virtual HRESULT EnumConnections(LPCGUID, LPVOID, LPVOID, uint32_t);
    virtual HRESULT EnumGroupsInGroup(DPXID, LPGUID, LPVOID, LPVOID, uint32_t);
    virtual HRESULT GetGroupConnectionSettings(uint32_t, DPXID, LPVOID, uint32_t*);
    virtual HRESULT InitializeConnection(LPVOID, uint32_t);
    virtual HRESULT SecureOpen(LPVOID, uint32_t, LPVOID, LPVOID);
    virtual HRESULT SendChatMessage(DPXID, DPXID, uint32_t, LPVOID);
    virtual HRESULT SetGroupConnectionSettings(uint32_t, DPXID, LPVOID);
    virtual HRESULT StartSession(uint32_t, DPXID);
    virtual HRESULT GetGroupFlags(DPXID, uint32_t*);
    virtual HRESULT GetGroupParent(DPXID, DPXID*);
    virtual HRESULT GetPlayerAccount(DPXID, uint32_t, LPVOID, uint32_t*);
    virtual HRESULT GetPlayerFlags(DPXID, uint32_t*);
    /* IDirectPlay4 */
    virtual HRESULT GetGroupOwner(DPXID, DPXID*);
    virtual HRESULT SetGroupOwner(DPXID, DPXID);
    virtual HRESULT SendEx(DPXID, DPXID, uint32_t, LPVOID, uint32_t,
                           uint32_t, uint32_t, LPVOID, uintptr_t*);
    virtual HRESULT GetMessageQueue(DPXID, DPXID, uint32_t, uint32_t*, uint32_t*);
    virtual HRESULT CancelMessage(uint32_t, uint32_t);
    virtual HRESULT CancelPriority(uint32_t, uint32_t, uint32_t);

    CDPlayNet();
    ~CDPlayNet();

    /* callback shapes (real DPLAY.H, function ptrs passed as LPVOID) */
    typedef BOOL (WINAPI *EnumConnCb)(LPCGUID, LPVOID, DWORD, LPVOID, DWORD, LPVOID);
    typedef BOOL (WINAPI *EnumSessCb)(LPVOID, LPDWORD, DWORD, LPVOID);

private:
    struct Msg {
        DPXID from, to;
        std::vector<uint8_t> data;
    };
    struct Conn {
        ENetPeer* peer = nullptr;
        std::set<DPXID> players;      /* remote player ids owned by conn */
        sockaddr_in addr{};
    };
    /* outbound packet queued for the ENet service thread — all enet_* calls
       must run on that thread (ENet hosts are not thread-safe). */
    struct OutPkt {
        ENetPeer* peer;
        std::vector<uint8_t> buf;
    };

    /* state */
    std::atomic<ULONG> refs{1};
    std::mutex mtx;
    bool initConn = false;            /* InitializeConnection called */
    bool isHost = false;
    bool inSession = false;
    std::atomic<bool> shutting{false};

    /* local players: id -> event handle (0 if none) */
    std::map<DPXID, HANDLE> localPlayers;
    std::map<DPXID, std::set<DPXID> > groups;   /* group -> members */
    std::set<DPXID> localGroups;
    std::deque<Msg> queue;

    DPXID nextId = 0x100;
    RDPSD2 sessionDesc{};             /* real-layout copy */
    std::string sessionName;

    /* ENet transport — server host when isHost, client host otherwise.
       One service thread owns the host: it drains outQ and services events. */
    ENetHost* eHost = nullptr;
    std::thread serviceThr;
    std::deque<OutPkt> outQ;
    std::mutex outMtx;
    bool draining = false;          /* teardown: drain then disconnect */

    /* host-side */
    std::vector<std::shared_ptr<Conn> > conns;
    int udpSrvFd = -1;              /* discovery responder */
    std::thread udpThr;

    /* client-side */
    std::shared_ptr<Conn> hostConn;
    std::map<uint32_t, std::pair<DPXID,bool> > pendingIds;  /* reqTag -> {id,done} */
    std::condition_variable idCv;
    std::condition_variable connCv;
    bool connectDone = false;       /* CONNECT event arrived */
    bool connectFailed = false;     /* connect timed out/refused */
    uint32_t reqTagSeq = 1;

    /* discovery (querier) — non-blocking async enum: each EnumSessions call
       drains replies into discHosts and reports all unexpired entries. */
    struct DiscHost {
        sockaddr_in sa;
        GUID inst, app;
        uint32_t flags, maxP, curP;
        char name[64];
        uint64_t lastSeenMs;
    };
    int udpQFd = -1;
    std::map<std::string, DiscHost> discHosts;      /* guidInstance text -> host */
    uint64_t lastQueryMs = 0;                       /* rate-limit query sends */

    /* internals — all called with mtx held unless noted */
    void enqueue(DPXID from, DPXID to, const void* data, size_t len);
    void enqueueSysMsg(const void* blob, size_t len);
    void queuePacket(ENetPeer* peer, uint32_t type, DPXID from, DPXID to,
                     uint32_t aux, const void* data, size_t len);
    HRESULT sendMsgToWire(Conn* c, DPXID from, DPXID to, const void* data, size_t len);
    HRESULT routeFromLocal(DPXID from, DPXID to, const void* data, size_t len);
    HRESULT hostRoute(Conn* src, DPXID from, DPXID to, const uint8_t* data, size_t len);
    void hostDestroyRemotePlayers(Conn* c);
    void enetServiceLoop();
    void handleEnetEvent(const ENetEvent& ev);
    void hostOnPacket(Conn* c, const uint8_t* p, size_t n);
    void clientOnPacket(const uint8_t* p, size_t n);
    void gracefulDisconnect();
    void udpResponderLoop();
    HRESULT requestId(DPXID* out, uint32_t kind);
    void pushSysCreate(DPXID id, uint32_t ptype);
    void pushSysDestroy(DPXID id, uint32_t ptype);
    void teardown();
    bool connOwnsPlayer(Conn* c, DPXID p) const {
        return c->players.count(p) != 0;
    }
};

void* RowanDPlay_CreateInstance();
