//------------------------------------------------------------------------------
//Filename       impact.h
//System         
//Author         Robert Slater
//Date           Thu 25 Nov 1999
//Description    
//				Designed by Jim! Handles balloons....
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	IMPACT_Included
#define	IMPACT_Included

#define	DEFAULT_IMPACT 0









class	Impact
{
private:

	bool	CheapCol(ItemPtr, ItemPtr);									//RJS 12Sep00


public:

	SLong 	GetDamageLoc(ShapeNum, animptr&);
	void	CollideAircraft(AirStrucPtr, AirStrucPtr);
	void	HitGroupElement(ItemPtr,ItemPtr,SLong index=-1, SLong amount=8, SLong damagetype=0);//RJS 05Apr00
	bool	HitGroup(ItemPtr,ItemPtr,SLong amount=24, SLong damagetype=0);	//RJS 11Aug00
	void	HitAircraft(ItemPtr,ItemPtr, SLong amount=8, SLong damagetype=0);	//RJS 30/05/00
	void	TestForBalloon(ItemPtr);									//RJS 12Sep00



};

extern	Impact	Fake_Damage;									//RJS 05Apr00



#pragma pack(pop)
#endif
