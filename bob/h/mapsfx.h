//------------------------------------------------------------------------------
//Filename       mapsfx.h
//System         
//Author         Robert Slater
//Date           Mon 8 May 2000
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	MAPSFX_Included
#define	MAPSFX_Included

#define	DEFAULT_MAPSFX 0


enum	MapSFX
{
	SFX_NULL=0,
	SFX_PhoneIn,
	SFX_PhoneOut,
	SFX_TeleType,
	SFX_AlertRaid,
	SFX_AlertScramble,
	SFX_OfficeAmbience
};



#pragma pack(pop)
#endif
