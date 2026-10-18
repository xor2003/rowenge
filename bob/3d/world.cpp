/*
	 Battle of Britain
	 Copyright (C) 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at bob@rowansoftware.com a
better place to ask is http://www.simhq.com/ or even :-
http://www.simhq.com/cgi-bin/boards/cgi-bin/forumdisplay.cgi?action=topics&forum=Battle+of+Britain&number=40&DaysPrune=20&LastLogin=
*/

//------------------------------------------------------------------------------
//Filename       world.cpp
//System         
//Author         Paul.   
//Date           Wed 30 Aug 1995
//Description    
//------------------------------------------------------------------------------

	#include	"DOSDefs.h"
	#include	"myAngles.h"
	#include	"WorldInc.h"
	#include	"World.h"
	#include	"myError.h"
	#include	"TransIte.h"									//PD 11May96
	#include	"3dcom.h"										//RJS 21Aug96
	#include	"viewsel.h"
	#include	"ranges.h"
	#include	"missman2.h"
	#include	"bfnumber.h"
	#include	"persons2.h"

//#define	_TOFILE_


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WorldStuff
//Author		Paul.   
//Date			Tue 19 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
WorldStuff::WorldStuff()
{
	//Clear the sector list
	routelist=NULL;
	int	counter;

	for(counter = 0;counter<SECTOR_MAXNUM;counter++)
		sectorlist[counter]=(itemptr )NULL;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WorldStuff
//Author		Paul.   
//Date			Tue 19 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
WorldStuff::~WorldStuff()
{

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure	addtoworld
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 30 Aug 1995
//Modified	
//
//Description	Adds an item to the sector list
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void WorldStuff::AddToWorld(itemptr item)
{
	UWord 	sector_number;

	sector_number = GetSector(item);

#ifndef NDEBUG																				//CSB 9Aug00
#define CHECKNOTINLIST																		//CSB 9Aug00
#ifdef  CHECKNOTINLIST																		//CSB 9Aug00
	for(itemptr listitem = sectorlist[sector_number]; listitem; listitem = listitem->Next)	//CSB 9Aug00
		if(listitem == item)																//CSB 9Aug00
			INT3;	//ITEM IS ALREADY IN THIS SECTOR LIST									//CSB 9Aug00
#endif																						//CSB 9Aug00
#endif																						//CSB 9Aug00

	if(sectorlist[sector_number]==(itemptr )NULL)
		{
		//First in the list
		sectorlist[sector_number] = item;
		item->Next = (itemptr )NULL;
		}
	else
		{
		//There are already some items in the list
		item->Next = sectorlist[sector_number];
		sectorlist[sector_number] = item;
		}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure	removefromworld
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 30 Aug 1995
//Modified		Split for moveall								//JIM 15Feb96
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void WorldStuff::RemoveFromWorld(itemptr item)
{

	RemoveFromSector(item,GetSector(item));
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RemoveFromSector
//Author		Jim Taylor
//Date			Thu 15 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	WorldStuff::RemoveFromSector(itemptr	item,UWord	sector_number)
{
	itemptr	next_item,
			curr_item;

	if(sectorlist[sector_number ] == (itemptr )NULL)
		{
			_Error.EmitSysErr(__FILE__":Item not found in sector\n");
		return;
		}

	//Search this sector for the item we want to remove

	if(sectorlist[sector_number]==item)
		{
		//It's at the head of the list

		sectorlist[sector_number] = sectorlist[sector_number]->Next;

		}
	else
		{
		curr_item = sectorlist[sector_number];

		if (curr_item==(itemptr)NULL)
			{
				_Error.EmitSysErr(__FILE__":Item not found in sector\n");
		return;
			}

		next_item = curr_item->Next;

		while(next_item!=item)
		{
			if (next_item==(itemptr)NULL)
				{
					_Error.EmitSysErr(__FILE__":Item not found in sector\n");
		return;
				}

			curr_item = next_item;

			if((next_item = curr_item->Next)==(itemptr )NULL)
			{
#ifdef	_TOFILE_
				FILE*	fp = fopen("worldbug.txt","wt");
				if (fp)
				{
					int	movecode = -1;
					int	theSag = -1;
					if (item->Status.size >= MOBILESIZE)
					{
						movecode = MobileItemPtr(item)->movecode;
						if (item->Status.size == AIRSTRUCSIZE)
						{
							if (AirStrucPtr(item)->fly.expandedsag)
								theSag = AirStrucPtr(item)->fly.expandedsag->uniqueID.count;
						}
					}

					fprintf(fp,"UID: %x\tSGT: %x\tMOVE: %d\tSAGUID: %x\n",int(item->uniqueID.count),int(item->SGT),movecode,theSag);

					fclose(fp);
				}
#endif
				_Error.EmitSysErr(__FILE__":Item not found in sector\n");
		return;
			}
		}

		//Found the item we want to remove so remove it!

		curr_item->Next = next_item->Next;

		}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		getsectorno
//Author		Paul.   
//Date			Mon 25 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void WorldStuff::getsectornos(itemptr item,UWord &sector_x, UWord &sector_y)
{
	ULong	temp;

	temp = (item->World.X - TDXBASE)>>WORLD_2_SECTOR_SHIFT;
	sector_x = temp & SECTOR_NUMBER_MASK;

	temp = (item->World.Z - TDZBASE)>>WORLD_2_SECTOR_SHIFT;
	sector_y = temp & SECTOR_NUMBER_MASK;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		makeindex
//Author		Paul.   
//Date			Mon 25 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UWord WorldStuff::makeindex(UWord sector_x, UWord sector_y)
{
	return(((sector_y&SECTOR_NUMBER_MASK)<<5)+(sector_x&SECTOR_NUMBER_MASK));
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		getfirstitem
//Author		Paul.   
//Date			Mon 25 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
itemptr WorldStuff::getfirstitem(UWord sector_index)
{
	return(sectorlist[sector_index]);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SameSector
//Author		Paul.   
//Date			Fri 14 Jun 1996
//
//Description	This is not good enough...
//				When you reach the edge of the world, half the visible objects 
//				are in the next realsector
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool WorldStuff::SameSector(itemptr itemA, itemptr itemB)
{	//using unsigned compare to discard all neg cases
	if ((ULong)(itemA->World.X - itemB->World.X +REAL_SECTOR_STEP)
				>(ULong)(REAL_SECTOR_STEP+REAL_SECTOR_STEP))
		return (FALSE);

	if ((ULong)(itemA->World.Z - itemB->World.Z +REAL_SECTOR_STEP)
				>(ULong)(REAL_SECTOR_STEP+REAL_SECTOR_STEP))
		return (FALSE);


	return (TRUE);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure	getsector
//LastModified:	PD 25Jun96
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 30 Aug 1995
//Modified	
//
//Description	
//
//Inputs		ptr to item
//
//Returns	index into sector list
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UWord WorldStuff::GetSector(itemptr item)
{
	UWord	sector_x,
			sector_y;

	getsectornos(item,sector_x,sector_y);

	return(makeindex(sector_x,sector_y));
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SectorEmpty
//Author		Paul.   
//Date			Thu 20 Aug 1998
//------------------------------------------------------------------------------
Bool WorldStuff::SectorEmpty(COORDS3D& pos)
{
	SWord sx=((pos.X-TDXBASE)>>WORLD_2_SECTOR_SHIFT)&SECTOR_NUMBER_MASK;
	SWord sy=((pos.Z-TDZBASE)>>WORLD_2_SECTOR_SHIFT)&SECTOR_NUMBER_MASK;
	sy=(((sy&SECTOR_NUMBER_MASK)<<5)+(sx&SECTOR_NUMBER_MASK));
	return Bool(sectorlist[sy]==NULL);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetLaunchers
//Author		Robert Slater
//Date			Wed 21 Aug 1996
//
//Description	Go through all the items in all the sectors and set up animation
//				data if needed
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	WorldStuff::SetLaunchers()
{
	for (int sn=SECTOR_MAXNUM-1;sn>=0;sn--)
	{
		ItemPtr	next=sectorlist[sn];

		while (next)
		{
			if (next->Anim == NULL)
				SHAPE.SetAnimData(next,0);						//RJS 27Sep96

			next=next->Next;
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetLaunchers
//Author		Robert Slater
//Date			Wed 21 Aug 1996
//
//Description	Go through all the items in all the sectors and set up animation
//				data if needed
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ItemBasePtr		WorldStuff::ConvertPtrUID(UniqueID tmpUID)
{
	return(pItem[tmpUID]);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetLaunchers
//Author		Robert Slater
//Date			Wed 21 Aug 1996
//
//Description	Go through all the items in all the sectors and set up animation
//				data if needed
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ItemBase::operator delete(void* area)
{
	if (!area || area==(void*)&Nil())	return;
	ItemBasePtr	it=(ItemBasePtr)area;
	if (it->Status.size==ItemBaseSize)
		::delete(ItemBasePtr)it;
	else
	if (intptr_t(ItemPtr(it)->Next)==-1)
			switch (it->Status.size)
			{
			case	ItemSize:		::delete(info_itemS*) it;		return;
//			case	HdgSize:		delete(HdgItemPtr) it;	return;
			case	WayPointSize:	::delete(info_waypoint*) it;	return;
//			case	HPSize:			delete(HpItemPtr) it;	return;
//			case	RotatedSize:	delete(RotItemPtr)it;		return;
//			case	MovingSize:		delete(MovingItemPtr)it;	return;
			case	MobileSize:		::delete(info_grndgrp*)it;	return;
//			case	TransientSize:	delete(TransientItemPtr)it;	return;
//			case	FormationSize:	delete(FormationItemPtr)it;	return;
			case	AirStrucSize:	::delete(info_airgrp*)it;	return;
//			case	ShipSize:		delete(ShipItemPtr)it;	return;
			default:
				_Error.EmitSysErr("Unexpected item type deleted!");
			}
	else
			switch (it->Status.size)
			{
			case	ItemSize:		::delete(ItemPtr) it;		return;
			case	HdgSize:		::delete(HdgItemPtr) it;	return;
			case	WayPointSize:	::delete(WayPointPtr) it;	return;
			case	HPSize:			::delete(HpItemPtr) it;	return;
			case	RotatedSize:	::delete(RotItemPtr)it;		return;
			case	MovingSize:		::delete(MovingItemPtr)it;	return;
			case	MobileSize:		::delete(MobileItemPtr)it;	return;
			case	TransientSize:	::delete(TransientItemPtr)it;	return;
			case	FormationSize:	::delete(FormationItemPtr)it;	return;
			case	AirStrucSize:	::delete(AirStrucPtr)it;	return;
			case	ShipSize:		::delete(ShipItemPtr)it;	return;
			}
			
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MakeSunPos
//Author		Robert Slater
//Date			Mon 23 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	WorldStuff::MakeSunPos(ViewPoint*	View_Point)
{
	SWord	sinang,cosang;
	ANGLES	sunang;

	if (View_Point)
	{
		int	currtime = View_Point->TimeOfDay() / 100;

		sunang=(Angles)(((currtime-MissManDawnTime())<<15)/(MissManDuskTime()-MissManDawnTime()));
	}
	else sunang = ANGLES_45Deg;								

	sunrot=sunang;
	Math_Lib.high_sin_cos(sunang,sinang,cosang);
	sunpos.Y=MULSHSIN(sinang,SUNELEVDIST,ANGLES_SHIFT);
	sunpos.X=MULSHSIN(cosang,SUNELEVDIST,ANGLES_SHIFT);
	Math_Lib.high_sin_cos(ANGLES_50Deg,sinang,cosang);
	sunpos.Z=-MULSHSIN(sinang,SUNELEVDIST,ANGLES_SHIFT);
	SLong	range;
	Math_Lib.Intercept(sunpos.X,sunpos.Y,sunpos.Z,range,sunhdg,sunpitch);
}

void	WorldStuff::SetViewpoint(ViewPoint* v) {vp=v;}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		NextItem
//Author		Robert Slater
//Date			Tue 11 Jan 2000
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	WorldStuff::NextItem(SLong& sector_no, ItemPtr&	theItem)
{
	bool	valid = true;
	if (sector_no == -1)
		theItem = NULL;

	if (theItem)
		theItem = theItem->Next;

	while (!theItem)
	{
		if (sector_no < (SECTOR_MAXNUM-1))
		{
			sector_no++;
			theItem = sectorlist[sector_no];
		}
		else
		{
			valid = false;
			break;
		}
	}

	return valid;
}
