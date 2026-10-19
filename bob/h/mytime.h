//------------------------------------------------------------------------------
//Filename       time.h
//System         
//Author         R. Hyde 
//Date           Thu 4 Jan 1996
//				 Code now in hardware\dostime&wintime.cpp		//ARM 01Jul96
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	TIME_Included
#define	TIME_Included

#include	"myangles.h"
#define	DEFAULT_TIME 0
enum	Times
{

		SECS100 = 100,							//RDH 20May96
		THIRTYSECS100 = 3000,
		MINS100	=	6000,
		FIVESECS100 = 5*(int)SECS100,							//RDH 20May96

 	SECSPERMIN = 60, 
 	SECSPERHR  = 60 * (int)SECSPERMIN,
 	SECSPERDAY = 24 * (int)SECSPERHR,
	SECSPERWEEK = 7 * (int)SECSPERDAY,
	SECSPERSAGFRAME=20,												  //JIM 03/12/99
 	DAYSPERYEAR = 365,
 	DAYSPERLEAPYEAR = 366,
 	DAYSPER4YEAR = 3 * (int)DAYSPERYEAR + (int)DAYSPERLEAPYEAR,

 	JAN1940 = ((int)SECSPERDAY * (((int)DAYSPER4YEAR * 9 ) + ((int)DAYSPERYEAR*3))),
	JUL1940	= (int)JAN1940 + (int)SECSPERDAY * (31+29+31+30+31+30),
	JUL10th1940 =  (int)JUL1940 + 9 * (int)SECSPERDAY,
	JUL15th1940 =  (int)JUL1940 + 14 * (int)SECSPERDAY,
	JUL20th1940 =  (int)JUL1940 + 19 * (int)SECSPERDAY,
	JUL25th1940 =  (int)JUL1940 + 24 * (int)SECSPERDAY,
	JUL31st1940 =  (int)JUL1940 + 30 * (int)SECSPERDAY,
	AUG1940	= (int)JAN1940 + (int)SECSPERDAY * (31+29+31+30+31+30+31),
	SEP1940	= (int)JAN1940 + (int)SECSPERDAY * (31+29+31+30+31+30+31+31),
	AUG11th1940 =  (int)AUG1940 + 10 * (int)SECSPERDAY,
	AUG12th1940 =  (int)AUG1940 + 11 * (int)SECSPERDAY,
	AUG15th1940 =  (int)AUG1940 + 14 * (int)SECSPERDAY,					//MS 11Nov00
	AUG16th1940 =  (int)AUG1940 + 15 * (int)SECSPERDAY,
	AUG23rd1940 =  (int)AUG1940 + 22 * (int)SECSPERDAY,
	AUG24th1940 =  (int)AUG1940 + 23 * (int)SECSPERDAY,
	AUG26th1940 =  (int)AUG1940 + 25 * (int)SECSPERDAY,
	AUG28th1940 =  (int)AUG1940 + 27 * (int)SECSPERDAY,
	AUG30th1940 =  (int)AUG1940 + 29 * (int)SECSPERDAY,
	SEP2nd1940 =  (int)SEP1940 + 1 * (int)SECSPERDAY,
	SEP4th1940 =  (int)SEP1940 + 3 * (int)SECSPERDAY,
	SEP6th1940 =  (int)SEP1940 + 5 * (int)SECSPERDAY,
	SEP7th1940 =  (int)SEP1940 + 6 * (int)SECSPERDAY,
	SEP10th1940 =  (int)SEP1940 + 9 * (int)SECSPERDAY,
	SEP15th1940 =  (int)SEP1940 + 14 * (int)SECSPERDAY,
 	SEP16th1940 =  (int)SEP1940 + 15 * (int)SECSPERDAY,


		MIN5		=	5* (int)SECSPERMIN,							  //JIM 19/01/99
		MIN10		= 10 * (int)SECSPERMIN,					//MS 11Jul96
		MIN15		= 15 * (int)SECSPERMIN,					//MS 11Jul96
		MIN20		= 20 * (int)SECSPERMIN,					//MS 11Jul96
		MIN30		= 30 * (int)SECSPERMIN,					//MS 11Jul96
		MIN40		= 40 * (int)SECSPERMIN,
		MIN50		= 50 * (int)SECSPERMIN,					//MS 11Jul96
		HR00		= 0,
		HR01		= 1 * (int)SECSPERHR,
		HR02		= 2 * (int)SECSPERHR,
		HR03		= 3 * (int)SECSPERHR,
		HR04		= 4 * (int)SECSPERHR,
		HR05		= 5 * (int)SECSPERHR,
		HR06		= 6 * (int)SECSPERHR,
		HR07		= 7 * (int)SECSPERHR,
		HR08		= 8 * (int)SECSPERHR,
		HR09		= 9 * (int)SECSPERHR,
		HR10		= 10 *(int) SECSPERHR,
		HR11		= 11 *(int) SECSPERHR,
		HR12		= 12 *(int) SECSPERHR,
		HR13		= 13 *(int) SECSPERHR,
		HR14		= 14 *(int) SECSPERHR,
		HR15		= 15 *(int) SECSPERHR,
		HR16		= 16 *(int) SECSPERHR,
		HR17		= 17 *(int) SECSPERHR,
		HR18		= 18 *(int) SECSPERHR,
		HR19		= 19 *(int) SECSPERHR,
		HR20		= 20 *(int) SECSPERHR,
		HR21		= 21 *(int) SECSPERHR,
		HR22		= 22 *(int) SECSPERHR,
		HR23		= 23 *(int) SECSPERHR,

		HR04MIN30	= 	(int)HR04 + (int)MIN30,							//MS 08Nov96
		HR05MIN40	=	(int)HR05 + (int)MIN40,
		HR08MIN20	=	(int)HR08 + (int)MIN20,
		HR08MIN50	=	(int)HR08 + (int)MIN50,
		HR09MIN50	=	(int)HR09 + (int)MIN50,
		HR09MIN30	=	(int)HR09 + (int)MIN30,							//MS 17Nov96
		HR12MIN10	=	(int)HR12 + (int)MIN10,
		HR12MIN15	=	(int)HR12 + (int)MIN15,
		HR12MIN20	=	(int)HR12 + (int)MIN20,
		HR14MIN20	=	(int)HR14 + (int)MIN20,
		HR14MIN30	=	(int)HR14 + (int)MIN30,
		HR14MIN50	=	(int)HR14 + (int)MIN50,
		HR15MIN20	=	(int)HR15 + (int)MIN20,	
		HR15MIN30	=	(int)HR15 + (int)MIN30,					//MS 11Jul96
		HR16MIN20	=	(int)HR16 + (int)MIN20,
		HR16MIN40	=	(int)HR16 + (int)MIN40,
		CRCEVENING = (int)HR20,
		MIDMORNING = (int)HR10,
		SO_EVENING = (int)HR19,
		STANDARDMORNING = (int)HR10 +(int) MIN30,
		MAYPOSTDAWNTIME = (int)HR05 +(int) MIN15,
		MORNINGPERIODSTART = (int)HR06 +(int) MIN30,
		MIDDAYPERIODSTART = HR11,
		AFTERNOONPERIODSTART = (int)HR15 +(int) MIN30,
		AFTERNOONPERIODEND = (int)HR20,
		TIME_TAKEOFF_ROUNDING	=	MIN5,
		TIME_TAKEOFF_FLIGHT		=	14,
		

	};




///////////////////////////////////////////////////////////////////////

// I have done it like this because of the amount of references to Timer_Code.FRAMETIME,
// if somebody wants to change them all and just have a const value without the TimerCode
// class, then fine. //AMM 25/11/99

class TimerCode
{
public:

	TimerCode()
		:FRAMETIME(4)
	{}

	long		FRAMETIME;
};

extern class TimerCode Timer_Code;

#pragma pack(pop)
#endif
