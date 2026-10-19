//------------------------------------------------------------------------------
//Filename       tilemake.h
//System         
//Author         Paul.   
//Date           Thu 16 Jul 1998
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	TILEMAKE_Included
#define	TILEMAKE_Included

#define	DEFAULT_TILEMAKE 0


struct DataRecord;

struct Rect
{
	SLong t,b,l,r;
};

const SLong _maxClips=5;

struct ClipInfo
{
	UByte level;
	UByte clipRecIndex;
};

struct SquareClips
{
	UWord count;
	ClipInfo clipInfo[_maxClips];
};

//struct ROWANSURFACEDESC;

class TMake
{
public:
	TMake();
	~TMake();

	void Init(const SLong,const SLong,CLib3D*) const;
	void Reinit(const SLong,const SLong,CLib3D*) const;
	void Exit() const;
	void ChkUpdate(const SLong,const SLong) const;
	DataRecord GetBlock(const SLong x,const SLong z) const;
	void FrameInit(WorldStuff *wP,LightVec& sunLight);					//JON 4/14/00
	void Render2Surface(const HTEXT&,UByte,UWord);
	void GetMinMaxAlt(const SLong x,const SLong z,const SWord dir,SLong& min,SLong& max);
	bool CheckCache();
	ImageMapDescPtr GetImageFromArea(UByte);
};

extern TMake* tMake;

#pragma pack(pop)
#endif
