


VOID MainLoopThread (PVOID pvoid);
VOID TimerThread (PVOID pvoid);
void CALLBACK TimeProc (UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

Bool GetCDFile (FileNum, char* Name, ULong Offset);
void ThreadFunction (void * ptr);

//TempCode ARM 09Aug96 // Temporary Debug params
//TempCode ARM 09Aug96 SLong Count;
//TempCode ARM 09Aug96 SLong MainLoopCount;
//TempCode ARM 09Aug96 // Temporary String for Message Boxes

// Direct Play Callback Functions
//TempCode ARM 13Aug96 BOOL FAR PASCAL EnumSP(LPGUID lpSPGuid, LPSTR lpDesc, DWORD dwMajorVersion, DWORD dwMinorVersion, LPVOID lpv);
//TempCode ARM 13Aug96 BOOL FAR PASCAL EnumSession(LPDPSESSIONDESC lpDPGameDesc, LPVOID lpContext, LPDWORD lpdwTimeOut, DWORD dwFlags);
//TempCode ARM 13Aug96 BOOL FAR PASCAL EnumPlayer(DPID dpID, LPSTR lpFriendlyName, LPSTR lpFormalName, DWORD dwFlags, LPVOID lpContext);

//TempCode ARM 15Aug96 BOOL FAR PASCAL EnumAllPlayers(DPID dpID, LPSTR lpFriendlyName, LPSTR lpFormalName, DWORD dwFlags, LPVOID lpContext)




