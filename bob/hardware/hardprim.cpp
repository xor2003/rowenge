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
//Filename       hardprim.cpp
//System         
//Author         Paul.   
//Date           Mon 3 Jul 1995
//Description    This is the hardware primitives library and provides the 
//			   following...
//
//				1) Set graphics mode
//				2) Screen swap + update w/o screenswap debug entry point
//				3) Single pixel plot to physical
//				4) Set text mode
//
//------------------------------------------------------------------------------


#include	<dos.h>

#include	"DosDefs.H"
#include "Palette.H"
#include	"Display.h"
#include	"myError.h"

extern	"C"	void		ASM_CopyMainRAM2VidRAM(char *, char *);
extern	"C"	void		ASM_CopyMainRAM2VidRAMF(char *, char *);
extern	"C"	void		ASM_SetPalette(char *);
extern	"C"	void		ASM_WaitVBL(void );
extern	"C"	void		ASM_OutDxAx(int ,int);
extern	"C"	void		ASM_OutDxAl(int ,int);
extern	"C"	short	ASM_PlotPixel(long, long, long, short);





//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure	HardPrim::PlotPixel
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
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
Colour Display::PlotPixel(SWord X, SWord Y, Colour colour)
{
	short	MinX,MaxX,MinY,MaxY;
	short	ret_val = colour;
	long		screenbase;
	long		yoffset,xoffset;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;

	if(X>=MinX && X<=MaxX && Y>=MinY && Y<=MaxY)
	{
		yoffset = BytesPerScanLine * Y;
		xoffset = BytesPerPixel * X;
		screenbase = VESA.writeaddr;//screenlist.Physical;

		ret_val = ASM_PlotPixel(screenbase,xoffset,yoffset,colour);
	}
	return((Colour)ret_val);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure	HardPrim::ClearScreen
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
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
void Display::ClearScreen(short colour)
{
	short	wx1,wx2,wy1,wy2;
	short	w,h;

	wx1 = PhysicalMinX;
	wx2 = wx1 + PhysicalWidth;

	wy1 = PhysicalMinY;
	wy2 = wy1 + PhysicalHeight;

	for(h=wy1;h<wy2;h++)
		for(w=wx1;w<wx2;w++)
			PlotPixel(w,h,Colour(colour));

}

