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

// RowanDialog: // SControl.cpp : implementation filez
//

#include "stdafx.h"
#include "mig.h"
#include "SControl.h"
#include	"savegame.h"
#include	"dinput.h"
#include	"comms.h"
#include	"winmove.h"
#include	"stub3d.h"

#ifdef _DEBUG
#ifndef THIS_FILE_DEFINED
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif
/////////////////////////////////////////////////////////////////////////////
//	enum	AllowAliasing	{AA_NONE,AA_GUNNER_JOYVIEW,AA_THROTTLE2_PROP1};

Analogue::AliassingOption
	Analogue::Aliassing_List[]={
					 //the following are primary axes and are used to maintain the SController screen
					{AA_THROTTLE2_PROP1,	AU_PROPPITCH,	IDS_PPITCH,		AU_THROTTLE2},
					{AA_NONE},
					{AA_GUNNER_JOYVIEW,	AU_AILERON,		IDS_ELEVATOR,		AU_GUNH},
					{AA_GUNNER_JOYVIEW,	AU_VIEWH,		IDS_VIEWPAN,		AU_GUNH},
					{AA_GUNNER_JOYVIEW,	AU_UI_X,		IDS_COCKPIT,		AU_GUNH},
					{AA_NONE},
					{AA_UI_PAN,		AU_VIEWH,		IDS_SHIFTEDINPUT,	AU_UI_X},
					{AA_NONE},	
					//the following are all the 'second' axes which should correspond to the primaries above
					{AA_GUNNER_JOYVIEW,	AU_ELEVATOR,	IDS_NONE,			AU_GUNP},
					{AA_GUNNER_JOYVIEW,	AU_VIEWP,		IDS_NONE,			AU_GUNP},
					{AA_GUNNER_JOYVIEW,	AU_UI_Y,		IDS_NONE,			AU_GUNP},
					{AA_UI_PAN,		AU_VIEWP,		IDS_NONE,	AU_UI_Y},
					{AA_NONE,AU_ILLEGAL}
					},
	*Analogue::Aliassing_Table[]={NULL,&Aliassing_List[2],&Aliassing_List[0],&Aliassing_List[6],NULL};



struct	devdesc
{
	CString name;
	CString	prodname;
	int		numbuttons,
			numhats,
			numaxes;
	bool	enablepoll,
			useff,
			ismouse,
			supportsff;
	GUID	devid;
};

devdesc			connecteddevices[20];

struct	axisdesc
{
	CString	name;
	bool	ishat,
			firstofpair,
			xtype;
	bool	ismouse;
	int		devnum;
	int		axisnum;
	AxisUses
			assignedtask;
};
axisdesc	connectedaxes[100];
int useddevices,usedaxes,firstaxes;
int joystickalias,mousealias;

axisdesc	shiftedaxis;
bool		shiftedaxisvalid=false;

int currdevaxiscount,currdevhatcount;

static	BOOL CALLBACK DIEnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE ddoi,LPVOID pvRef)
{	
	//How shiftedaxis works:
	//	At various points we fill in shiftedaxis record,
	//	then we add it after the next valid record
	//
	//	New rewrite calls for only supporting shiftedaxis for X/Y on MOUSE
	//



	connectedaxes[usedaxes].name=ddoi->tszName;
	connectedaxes[usedaxes].devnum=useddevices;
	if (ddoi->guidType==GUID_POV)
	{
		connectedaxes[usedaxes].ishat=true;					 //add this axis properly
		connectedaxes[usedaxes].axisnum=currdevhatcount++;
		connectedaxes[usedaxes].assignedtask=AU_UNASSIGNED;
		connectedaxes[usedaxes].xtype=true;
		connectedaxes[usedaxes].firstofpair=true;
		if (connecteddevices[useddevices].ismouse)
		{
			connectedaxes[usedaxes].ismouse=true;
		}

	}
	else
	{
		connectedaxes[usedaxes].ishat=false;
		connectedaxes[usedaxes].axisnum=currdevaxiscount++;
		connectedaxes[usedaxes].assignedtask=AU_UNASSIGNED;
		if (ddoi->guidType==GUID_XAxis)
		{
			connectedaxes[usedaxes].xtype=true;
			connectedaxes[usedaxes].firstofpair=true;
		}
		else if (ddoi->guidType!=GUID_YAxis)
		{
			connectedaxes[usedaxes].xtype=true;
			connectedaxes[usedaxes].firstofpair=false;
		}
		else
		{
			connectedaxes[usedaxes].xtype=false;
			connectedaxes[usedaxes].firstofpair=false;
		}
		if (connecteddevices[useddevices].ismouse)
		{
			connectedaxes[usedaxes].ismouse=true;

// master mouse - create extra axes

		}
	}
	usedaxes++;
	return DIENUM_CONTINUE;
}
static BOOL CALLBACK DIEnumDevicesProc(const DIDEVICEINSTANCE *ddi,LPVOID pvRef)
{
	if ((ddi->dwDevType&255)!=DIDEVTYPE_KEYBOARD)
	{
		LPDIRECTINPUT	DIdev=(LPDIRECTINPUT)pvRef;
		connecteddevices[useddevices].name=ddi->tszInstanceName;
		const char* prodname=ddi->tszProductName;
		while (strlen(prodname)>25)
			if (strchr(prodname,' '))
				prodname=strchr(prodname,' ')+1;
			else
				break;
		connecteddevices[useddevices].enablepoll=false;
		connecteddevices[useddevices].useff=false;
		connecteddevices[useddevices].prodname=prodname;
		connecteddevices[useddevices].devid=ddi->guidInstance;
		connecteddevices[useddevices].ismouse=((ddi->dwDevType&255)==DIDEVTYPE_MOUSE);
		LPDIRECTINPUTDEVICE dev;
		DIDEVCAPS			caps={sizeof(DIDEVCAPS)};
		DIdev->CreateDevice(connecteddevices[useddevices].devid,&dev,NULL);
		dev->GetCapabilities(&caps);
		connecteddevices[useddevices].supportsff=(caps.dwFlags&DIDC_FORCEFEEDBACK)!=0;
		connecteddevices[useddevices].numbuttons=caps.dwButtons;
		connecteddevices[useddevices].numhats=caps.dwPOVs;
		connecteddevices[useddevices].numaxes=caps.dwAxes;
		currdevaxiscount=0;
		currdevhatcount=0;
		dev->EnumObjects(DIEnumDeviceObjectsProc,NULL,DIDFT_AXIS+DIDFT_POV);
		dev->Release();

		if (shiftedaxisvalid)
		{
// still got one axis to add

			shiftedaxisvalid=false;

			connectedaxes[usedaxes].name=shiftedaxis.name;
			connectedaxes[usedaxes].devnum=shiftedaxis.devnum;

			connectedaxes[usedaxes].ismouse=shiftedaxis.ismouse;
			connectedaxes[usedaxes].ishat=shiftedaxis.ishat;

			if (connectedaxes[usedaxes].ishat)
				connectedaxes[usedaxes].axisnum=currdevhatcount++;
			else
				connectedaxes[usedaxes].axisnum=currdevaxiscount++;

			connectedaxes[usedaxes].assignedtask=AU_UNASSIGNED;
			connectedaxes[usedaxes].xtype=shiftedaxis.xtype;
			connectedaxes[usedaxes].firstofpair=shiftedaxis.firstofpair;
			connectedaxes[usedaxes].ismouse=shiftedaxis.ismouse;

			usedaxes++;
		}


		useddevices++;
	}
	return DIENUM_CONTINUE;
}

static int	sameuseddevices=0;
static bool	differentdevices=false;
static bool donotremake=false;
static BOOL CALLBACK DIEnumDevicesSameProc(const DIDEVICEINSTANCE *ddi,LPVOID pvRef)
{
	if ((ddi->dwDevType&255)!=DIDEVTYPE_KEYBOARD)
	{
		LPDIRECTINPUT	DIdev=(LPDIRECTINPUT)pvRef;
		if (strcmp(connecteddevices[sameuseddevices].name,ddi->tszInstanceName)!=0)
			differentdevices=true;
		const char* prodname=ddi->tszProductName;
		while (strlen(prodname)>25)
			if (strchr(prodname,' '))
				prodname=strchr(prodname,' ')+1;
			else
				break;
		if (strcmp(connecteddevices[sameuseddevices].prodname,prodname)!=0)
			differentdevices=true;
		if (connecteddevices[sameuseddevices].devid!=ddi->guidInstance)
			differentdevices=true;
		if (connecteddevices[sameuseddevices].ismouse!=((ddi->dwDevType&255)==DIDEVTYPE_MOUSE))
			differentdevices=true;
		LPDIRECTINPUTDEVICE dev;
		DIDEVCAPS			caps={sizeof(DIDEVCAPS)};
		int rv=DIdev->CreateDevice(ddi->guidInstance,&dev,NULL);
		if (rv!=DI_OK)
			donotremake=true;
		else
		{
			dev->GetCapabilities(&caps);
			if (connecteddevices[sameuseddevices].supportsff!=((caps.dwFlags&DIDC_FORCEFEEDBACK)!=0))
				differentdevices=true;
			if (connecteddevices[sameuseddevices].numbuttons!=caps.dwButtons)
				differentdevices=true;
			if (connecteddevices[sameuseddevices].numhats!=caps.dwPOVs)
				differentdevices=true;
			if (connecteddevices[sameuseddevices].numaxes!=caps.dwAxes)
				differentdevices=true;
			dev->Release();
		}

		sameuseddevices++;
	}
	return DIENUM_CONTINUE;
}


static bool	GotSameDevices()
{
	sameuseddevices=2;
	differentdevices=false;
	donotremake=false;
	LPDIRECTINPUT	DIdev;
	DirectInputCreate(AfxGetInstanceHandle( ),DIRECTINPUT_VERSION,&DIdev,NULL);
	DIdev->EnumDevices(0,DIEnumDevicesSameProc,DIdev,DIEDFL_ATTACHEDONLY);

	DIdev->Release();  
	if (donotremake)
		return true;
	if (differentdevices)
		return false;
	int	i;
	for (i=2;i<sameuseddevices;i++)
		breakif (!connecteddevices[i].ismouse);
	if (i<sameuseddevices)	//copy first joystick as default
	{
		if (joystickalias!=i)
			return false;
	}
	else
	{
		if (connecteddevices[0].devid.Data1!=0)
			return false;
	}
	for (i=2;i<sameuseddevices;i++)
		breakif (connecteddevices[i].ismouse);
	if (i<sameuseddevices)	//copy first joystick as default
	{
		if (mousealias!=i)
			return false;
	}
	else
	{
		if (connecteddevices[1].devid.Data1!=0)
			return false;
	}
	
	if (sameuseddevices<=4)	//2 reserved general + 2 specific
	{
		sameuseddevices=2;
	}
	if (sameuseddevices!=useddevices)
		return false;
	return true;


}
void	SController::BuildEnumerationTables()
{
	useddevices=2;
	const int middleaxes=20;
	usedaxes=middleaxes;
	firstaxes=middleaxes;
	LPDIRECTINPUT	DIdev;
	DirectInputCreate(AfxGetInstanceHandle( ),DIRECTINPUT_VERSION,&DIdev,NULL);
	DIdev->EnumDevices(0,DIEnumDevicesProc,DIdev,DIEDFL_ATTACHEDONLY);
	int i;
	joystickalias=0;
	mousealias=0;
	for (i=2;i<useddevices;i++)
		breakif (!connecteddevices[i].ismouse);
	if (i<useddevices)	//copy first joystick as default
	{
		joystickalias=i;
		connecteddevices[0]=connecteddevices[i];
		connecteddevices[0].name=RESSTRING(FIRSTJOYSTICK);
		for (int j=usedaxes;--j>10;)
			if (connectedaxes[j].devnum==i)
			{
				connectedaxes[--firstaxes]=connectedaxes[j];
				connectedaxes[firstaxes].devnum=0;
			}
	}
	else
	{
		connecteddevices[0].devid.Data1=0;
		connecteddevices[0].name=RESSTRING(FIRSTJOYSTICK);
		connecteddevices[0].prodname=RESSTRING(NOTCONNECTED);
		connecteddevices[0].numaxes=0;
		connecteddevices[0].numbuttons=0;
		connecteddevices[0].numhats=0;
	}
	DIdev->Release();

	for (i=2;i<useddevices;i++)
		breakif (connecteddevices[i].ismouse);
	if (i<useddevices)	//copy first joystick as default
	{
		mousealias=i;
		connecteddevices[1]=connecteddevices[i];
		connecteddevices[1].name=RESSTRING(FIRSTMOUSE);
		for (int j=usedaxes;--j>=10;)
			if (connectedaxes[j].devnum==i)
			{
				connectedaxes[--firstaxes]=connectedaxes[j];
				connectedaxes[firstaxes].devnum=1;
			}
	}
	else
	{
		connecteddevices[1].devid.Data1=0;
		connecteddevices[1].name=RESSTRING(FIRSTMOUSE);
		connecteddevices[1].prodname=RESSTRING(NOTCONNECTED);
		connecteddevices[1].numaxes=0;
		connecteddevices[1].numbuttons=0;
		connecteddevices[1].numhats=0;
	}
	
	if (useddevices<=4)	//2 reserved general + 2 specific
	{
		useddevices=2;
		usedaxes=middleaxes;
	}
}
/////////////////////////////////////////////////////////////////////////////
// SController dialog
SController::SController(CWnd* pParent /*=NULL*/)
	: RowanDialog(SController::IDD, pParent)
{
	//{{AFX_DATA_INIT(SController)
	//}}AFX_DATA_INIT
}


void SController::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SController)
	DDX_Control(pDX,IDC_RSTATICCTRL6,m_IDC_RSTATICCTRL6);
	DDX_Control(pDX,IDC_RSTATICCTRL9,m_IDC_RSTATICCTRL9);
	DDX_Control(pDX,IDC_RSTATICCTRL12,m_IDC_RSTATICCTRL12);
	DDX_Control(pDX,IDC_RSTATICCTRL14,m_IDC_RSTATICCTRL14);
	DDX_Control(pDX,IDC_RSTATICCTRL16,m_IDC_RSTATICCTRL16);
	DDX_Control(pDX,IDC_DEVLIST,m_IDC_DEVLIST);
	DDX_Control(pDX,IDC_STICKDEV,m_IDC_STICKDEV);
	DDX_Control(pDX,IDC_THROTDEV,m_IDC_THROTDEV);
	DDX_Control(pDX,IDC_RUDDEV,m_IDC_RUDDEV);
	DDX_Control(pDX,IDC_PANDEV,m_IDC_PANDEV);
	DDX_Control(pDX,IDC_RSTATICCTRL17,m_IDC_RSTATICCTRL17);
	DDX_Control(pDX,IDC_RSTATICCTRL18,m_IDC_RSTATICCTRL18);
	DDX_Control(pDX,IDC_RSTATICCTRL19,m_IDC_RSTATICCTRL19);
	DDX_Control(pDX,IDC_PITCHDEV,m_IDC_PITCHDEV);
	DDX_Control(pDX,IDC_ZOOMDEV,m_IDC_ZOOMDEV);
	DDX_Control(pDX,IDC_MOUSEDEV,m_IDC_MOUSEDEV);
	DDX_Control(pDX,IDC_DEVDESC,m_IDC_DEVDESC);
	DDX_Control(pDX,IDC_CALIB,m_IDC_CALIB);
	DDX_Control(pDX,IDC_SUPPFF,m_IDC_SUPPFF);
	DDX_Control(pDX,IDC_SELFF,m_IDC_SELFF);
	DDX_Control(pDX,IDC_SUPPFF2,m_IDC_SUPPFF2);
	DDX_Control(pDX,IDC_SELPOLL,m_IDC_SELPOLL);
	DDX_Control(pDX,IDC_FLIP_GUNNER,m_IDC_FLIP_GUNNER);
	DDX_Control(pDX,IDC_FLIP_COCKPIT, m_IDC_FLIP_COCKPIT);
	DDX_Control(pDX,IDC_FLIP_PAN, m_IDC_FLIP_PAN);
	DDX_Control(pDX,IDC_RSTATICCTRL7,m_IDC_RSTATICCTRL7);
	DDX_Control(pDX,IDC_RSTATICCTRL8,m_IDC_RSTATICCTRL8);
	DDX_Control(pDX,IDC_RSTATICCTRL46,m_IDC_RSTATICCTRL46);
	DDX_Control(pDX,IDC_CBO_FF_GUNRATTLE,m_IDC_CBO_FF_GUNRATTLE);
	DDX_Control(pDX,IDC_CBO_FF_AIRFRAME,m_IDC_CBO_FF_AIRFRAME);
	DDX_Control(pDX, IDC_CBO_STICKDZ, m_IDC_CBO_STICKDZ);
	DDX_Control(pDX, IDC_CBO_STICKMODE, m_IDC_CBO_STICKMODE);
	DDX_Control(pDX, IDC_CBO_RUDDZ, m_IDC_CBO_RUDDZ);
	DDX_Control(pDX, IDC_CBO_COCKPITMODE, m_IDC_CBO_COCKPITMODE);
	DDX_Control(pDX, IDC_CBO_GUNNERDZ, m_IDC_CBO_GUNNERDZ);
	DDX_Control(pDX, IDC_CBO_GUNNERMODE, m_IDC_CBO_GUNNERMODE);
	DDX_Control(pDX, IDC_CBO_PANDZ, m_IDC_CBO_PANDZ);
	DDX_Control(pDX, IDC_CBO_PANMODE, m_IDC_CBO_PANMODE);
	DDX_Control(pDX, IDC_CBO_PP2DZ, m_IDC_CBO_PP2DZ);
	DDX_Control(pDX, IDC_CBO_PPDZ, m_IDC_CBO_PPDZ);
	DDX_Control(pDX, IDC_CBO_RUDMODE, m_IDC_CBO_RUDMODE);
	DDX_Control(pDX, IDC_CBO_ZOOMDZ, m_IDC_CBO_ZOOMDZ);
	DDX_Control(pDX, IDC_CBO_ZOOMMODE, m_IDC_CBO_ZOOMMODE);
	DDX_Control(pDX, IDC_GUNNER, m_IDC_GUNNER);
	DDX_Control(pDX, IDC_PROPPITCH, m_IDC_PROPPITCH);
	DDX_Control(pDX, IDC_PROPPITCH2, m_IDC_PROPPITCH2);
	DDX_Control(pDX, IDC_THROTDEV_2, m_IDC_THROTDEV_2);
	DDX_Control(pDX, IDC_CBO_THROT2DZ, m_IDC_CBO_THROT2DZ);
	DDX_Control(pDX, IDC_CBO_THROTDZ, m_IDC_CBO_THROTDZ);
	DDX_Control(pDX, IDC_INTERACTIVECPIT, m_IDC_INTERACTIVECPIT);
	DDX_Control(pDX, IDC_CBO_FF_AERO, m_IDC_CBO_FF_AERO);
	DDX_Control(pDX, IDC_CBO_FF_BUFF, m_IDC_CBO_FF_BUFF);
	DDX_Control(pDX, IDC_FLIP_PANHORI, m_IDC_FLIP_PANHORI);
	//}}AFX_DATA_MAP
#define	SG2C_DISPLAY setlevel
#include "sg2combo.h"
	AfxGetMainWnd()->ModifyStyleEx(WS_EX_TOPMOST,0);
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);

	AfxGetMainWnd()->ModifyStyleEx(WS_EX_TOPMOST,0);
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);

}

void SController::PreDestroyPanel()
{
#define	SG2C_WRITEBACK setlevel
#include "sg2combo.h"
	SetAxisConfig(false,false);

	AfxGetMainWnd()->ModifyStyleEx(0,WS_EX_TOPMOST);
	AfxGetMainWnd()->ShowWindow(SW_SHOWMAXIMIZED);
}
#define SG2C_CLEANUP
#include "sg2combo.h"

BEGIN_MESSAGE_MAP(SController, CDialog)
	//{{AFX_MSG_MAP(SController)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(SController, CDialog)
    //{{AFX_EVENTSINK_MAP(SController)
	ON_EVENT(SController, IDC_CALIB, 1 /* Clicked */, OnClickedCalib, VTS_NONE)
	ON_EVENT(SController, IDC_DEVLIST, 1 /* TextChanged */, OnTextChangedDevlist, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_MOUSEDEV, 1 /* TextChanged */, OnTextChangedMousedev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_PANDEV, 1 /* TextChanged */, OnTextChangedPandev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_RUDDEV, 1 /* TextChanged */, OnTextChangedRuddev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_SELFF, 1 /* Clicked */, OnClickedSelff, VTS_NONE)
	ON_EVENT(SController, IDC_SELPOLL, 1 /* Clicked */, OnClickedSelpoll, VTS_NONE)
	ON_EVENT(SController, IDC_STICKDEV, 1 /* TextChanged */, OnTextChangedStickdev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_THROTDEV, 1 /* TextChanged */, OnTextChangedThrotdev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_ZOOMDEV, 1 /* TextChanged */, OnTextChangedZoomdev, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_STICKDZ, 1 /* TextChanged */, OnTextChangedCboStickdz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_STICKMODE, 1 /* TextChanged */, OnTextChangedCboStickdmode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_THROTDEV_2, 1 /* TextChanged */, OnTextChangedThrotdev2, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_PROPPITCH2, 1 /* TextChanged */, OnTextChangedProppitch2, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_PROPPITCH, 1 /* TextChanged */, OnTextChangedProppitch, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_INTERACTIVECPIT, 1 /* TextChanged */, OnTextChangedInteractivecockpit, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_GUNNER, 1 /* TextChanged */, OnTextChangedGunner, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_ZOOMMODE, 1 /* TextChanged */, OnTextChangedCboZoommode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_THROTDZ, 1 /* TextChanged */, OnTextChangedCboThrotdz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_THROT2DZ, 1 /* TextChanged */, OnTextChangedCboThrot2dz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_RUDMODE, 1 /* TextChanged */, OnTextChangedCboRudmode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_RUDDZ, 1 /* TextChanged */, OnTextChangedCboRuddz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_PPDZ, 1 /* TextChanged */, OnTextChangedCboPpdz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_PP2DZ, 1 /* TextChanged */, OnTextChangedCboPp2dz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_PANMODE, 1 /* TextChanged */, OnTextChangedCboPanmode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_PANDZ, 1 /* TextChanged */, OnTextChangedCboPandz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_GUNNERMODE, 1 /* TextChanged */, OnTextChangedCboGunnermode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_GUNNERDZ, 1 /* TextChanged */, OnTextChangedCboGunnerdz, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_COCKPITMODE, 1 /* TextChanged */, OnTextChangedCboCockpitmode, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_FF_GUNRATTLE, 1 /* TextChanged */, OnTextChangedCboFfGunrattle, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_FF_BUFF, 1 /* TextChanged */, OnTextChangedCboFfBuff, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_FF_AIRFRAME, 1 /* TextChanged */, OnTextChangedCboFfAirframe, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_CBO_FF_AERO, 1 /* TextChanged */, OnTextChangedCboFfAero, VTS_BSTR VTS_I2)
	ON_EVENT(SController, IDC_FLIP_COCKPIT, 1 /* Clicked */, OnClickedFlipCockpit, VTS_NONE)
	ON_EVENT(SController, IDC_FLIP_GUNNER, 1 /* Clicked */, OnClickedFlipGunner, VTS_NONE)
	ON_EVENT(SController, IDC_FLIP_PAN, 1 /* Clicked */, OnClickedFlipPan, VTS_NONE)
	ON_EVENT(SController, IDC_FLIP_PANHORI, 1 /* Clicked */, OnClickedFlipPanhori, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////

bool	SController::GetAxisConfig(bool setff)	//convert runtime device list to ui axis list
{
	bool donesome=false;
	for (int rtdev=0;_Analogue.runtimedevices[rtdev].devid.Data1;rtdev++)
	{
		int setdev=0;
		if (_Analogue.runtimedevices[rtdev].devid.Data1==1)
			setdev=0;
		else
		if (_Analogue.runtimedevices[rtdev].devid.Data1==2)
			setdev=1;
		else
		for (setdev=2;setdev<useddevices;setdev++)
			breakif(connecteddevices[setdev].devid==_Analogue.runtimedevices[rtdev].devid);

		if (setdev<useddevices)
		{
			donesome=true;
			connecteddevices[setdev].enablepoll=true;
			connecteddevices[setdev].useff=_Analogue.runtimedevices[rtdev].useff;
			int ra=0;
			int rh=0;
			int sra=0;

			for (int va=firstaxes;va<usedaxes;va++)
				if (connectedaxes[va].devnum==setdev)
				{
// if shifted then get from shifted axes - no shifted hats
					if (!strncmp("SH_",connectedaxes[va].name,3))
					{
						if (!connectedaxes[va].ishat)
							connectedaxes[va].assignedtask=(AxisUses)_Analogue.runtimedevices[rtdev].sh_axismaps[sra++];
						else
							connectedaxes[va].assignedtask=AU_UNASSIGNED;
					}
					else
					{
						if (connectedaxes[va].ishat)
							connectedaxes[va].assignedtask=(AxisUses)_Analogue.runtimedevices[rtdev].hatmaps[rh++];
						else
							connectedaxes[va].assignedtask=(AxisUses)_Analogue.runtimedevices[rtdev].axismaps[ra++];
					}
				}
		}
	}

	bool backup=false;
	if (!donesome)
	{
		SetDefaultStickSettings(setff);
		backup=true;
	}

	return backup;
}

void	SController::SetDefaultStickSettings(bool setff)
{
	if (connecteddevices[0].supportsff && setff)
		connecteddevices[0].useff=true;
	connecteddevices[0].enablepoll=true;
	connecteddevices[1].enablepoll=true;
	bool donestick=false,donethr=false,doneped=false,donecock=false;
	for (int va=firstaxes;va<usedaxes;va++)
	{
		if (connectedaxes[va].devnum==0)
		{
// first joystick

			if (connectedaxes[va].ishat)
			{
				;////	{if (!rh++)	connectedaxes[va].assignedtask=AU_VIEWH;}
			}
			else
			{
				if (connectedaxes[va].assignedtask==AU_UNASSIGNED)
				{
// dont really want to use shifted axes etc as defaults

					CString s;
					s.LoadString(IDS_SHIFTEDINPUT);

					if (connectedaxes[va].name.Find(s)==-1)
					{
						if (connectedaxes[va].firstofpair && !donestick)
						{
							connectedaxes[va].assignedtask=AU_AILERON;
							connectedaxes[va+1].assignedtask=AU_ELEVATOR;
							donestick=true;
						}
						else
// more likely to want throttle control first than rudder

						if (!donethr)
						{
							connectedaxes[va].assignedtask=AU_THROTTLE;
							donethr=true;
						}
						else
						if (connectedaxes[va].xtype && !doneped)
						{
							connectedaxes[va].assignedtask=AU_RUDDER;
							doneped=true;
						}
					}
				}
			}
		}
		else if (connectedaxes[va].devnum==1)
		{
// first mouse

			if (connectedaxes[va].ismouse && connectedaxes[va].firstofpair && !donecock)
			{

				connectedaxes[va].assignedtask=AU_VIEWH;
				connectedaxes[va+1].assignedtask=AxisUses(AU_VIEWP);
				_Analogue.tune[AU_VIEWH].aliased|=AA_GUNNER_JOYVIEW;
				_Analogue.tune[AU_VIEWP].aliased|=AA_GUNNER_JOYVIEW;
				_Analogue.tune[AU_VIEWH].aliased|=AA_UI_PAN;
				_Analogue.tune[AU_VIEWP].aliased|=AA_UI_PAN;

				donecock=true;
			}
		}
	}
}


void	SController::SetAxisConfig(bool mouseshift, bool joyshift)	//set runtime device config from ui axis list
{
	int rtdev=0;
	for (int setdev=0;setdev<useddevices;setdev++)
	{
		if (connecteddevices[setdev].enablepoll && connecteddevices[setdev].devid.Data1)
		{
			if (setdev==0)
				_Analogue.runtimedevices[rtdev].devid.Data1=1;
			else
			if (setdev==1)
				_Analogue.runtimedevices[rtdev].devid.Data1=2;
			else
				_Analogue.runtimedevices[rtdev].devid=connecteddevices[setdev].devid;
			_Analogue.runtimedevices[rtdev].useff=connecteddevices[setdev].useff;
			_Analogue.runtimedevices[rtdev].ismouse=connecteddevices[setdev].ismouse;
			int ra=0;
			int rh=0;

			for (int va=firstaxes;va<usedaxes;va++)
			{
				if (connectedaxes[va].devnum==setdev)
				{
					if (connectedaxes[va].ishat)
					{
						_Analogue.runtimedevices[rtdev].hatmaps[rh++]=connectedaxes[va].assignedtask;
					}	
					else
					{
						{
								_Analogue.runtimedevices[rtdev].axismaps[ra++]=connectedaxes[va].assignedtask;
						}
					}
				}
			}

			rtdev++;
		}
	}
	_Analogue.runtimedevices[rtdev].devid.Data1=0;
}


void	SController::RemakeDisplay()
{
	CRCombo* combo=GETDLGITEM(IDC_DEVLIST);
	int	index=0;//combo->GetIndex();									//AMM 22Aug00
	CString currdevname;
	currdevname=combo->GetCaption();


	combo->Clear();
	for (int i=0;i<useddevices;i++)
	{
		CString devname=connecteddevices[i].name;
		devname+=": ";
		devname+=connecteddevices[i].prodname;
		combo->AddString(CSprintf("%s: %s",connecteddevices[i].name,connecteddevices[i].prodname));	

		if (currdevname==devname)
			index=i;
	}
	combo->SetIndex(index);
	CRStatic* desc=GETDLGITEM(IDC_DEVDESC);
	desc->SetString(CSprintf("%i %s, %i %s, %i %s",
						connecteddevices[index].numaxes,RESSTRING(JOYAXES),
						connecteddevices[index].numhats,RESSTRING(COOLIEHATS),
						connecteddevices[index].numbuttons,RESSTRING(BUTTONS)	));
	CRButton* button=GETDLGITEM(IDC_SELPOLL);
	button->SetPressed(connecteddevices[index].enablepoll);
	button=GETDLGITEM(IDC_SELFF);
	desc=GETDLGITEM(IDC_SUPPFF);
	desc->SetString(RESSTRING(USEFFOUT));
	if (connecteddevices[index].supportsff)
	{
		desc->SetForeColor(RGB(255,255,0));

		button->SetDisabled(FALSE);
		button->SetPressed(connecteddevices[index].useff);
	}
	else
	{
		desc->SetForeColor(RGB(160,160,0));
		button->SetPressed(FALSE);
		button->SetDisabled(TRUE);
	}
	RemakeAxes();
}
void	SController::RemakeAxes()
{

//  xonly, analogonly, paironly, notstick, notmouse, allowshifted

	SetAxesCombo(m_IDC_STICKDEV,AU_AILERON,true,true,true,false,true,false,true,AA_AA_SETUP);
	SetAxesCombo(m_IDC_RUDDEV,AU_RUDDER,false,true,false,false,true,false,true);
	SetAxesCombo(m_IDC_THROTDEV,AU_THROTTLE,	false,true,false,false,false,false,true);
	SetAxesCombo(m_IDC_PROPPITCH,AU_PROPPITCH,	false,true,false,false,false,false,true,AA_AA_SETUP);
	SetAxesCombo(m_IDC_THROTDEV_2,AU_THROTTLE2,	false,true,false,false,false,false,true,AA_THROTTLE2_PROP1);
	SetAxesCombo(m_IDC_PROPPITCH2,AU_PROPPITCH2,false,true,false,false,false,false,true);

	SetAxesCombo(m_IDC_PANDEV,AU_VIEWH,false,false,true,false,false,true,true,AA_AA_SETUP);
	SetAxesCombo(m_IDC_INTERACTIVECPIT,AU_COCKPIT,true,false,true,true,false,true,true,AA_UI_PAN);
	SetAxesCombo(m_IDC_GUNNER,AU_GUNH,false,true,true,false,false,true,true,AA_GUNNER_JOYVIEW);
	
	SetAxesCombo(m_IDC_ZOOMDEV,AU_VIEWZ,false,true,false,true,false,false,true);
}

int	SController::ChangedAxesCombo(CRCombo& m_ID,AxisUses task,bool xonly,bool analogonly,bool paironly,bool notstick, bool notmouse, bool allowshifted, bool allownormal,AllowAliasing allowaliasing)
{
	int newindex;
	{
		CRCombo* combo=GETDLGITEM(ID);
		newindex=combo->GetIndex();
		combo->Clear();
	}
	int currindex=1;
	int selindex=-1;
	for (int i=firstaxes;i<usedaxes;i++)
	{
		if (connectedaxes[i].assignedtask==task)
		{
			connectedaxes[i].assignedtask=AU_UNASSIGNED;
			if (paironly)
				connectedaxes[i+1].assignedtask=AU_UNASSIGNED;
		}

		if (!connecteddevices[connectedaxes[i].devnum].enablepoll)
			continue;
		if (xonly && !connectedaxes[i].xtype)
			continue;
		if (analogonly && connectedaxes[i].ishat)
			continue;
		if (paironly && !connectedaxes[i].firstofpair)
			continue;
		if (connectedaxes[i].assignedtask!=AU_UNASSIGNED)// &&
//		(((task==AU_THROTTLE2 && connectedaxes[i].assignedtask!=AU_THROTTLE)
///		&& (task==AU_THROTTLE && connectedaxes[i].assignedtask!=AU_THROTTLE2)
///		&& (task==AU_PROPPITCH && connectedaxes[i].assignedtask!=AU_PROPPITCH2)
//		&& (task==AU_PROPPITCH2 && connectedaxes[i].assignedtask!=AU_PROPPITCH))))
		{
			continue;
		}
		if (notstick && !connectedaxes[i].ismouse)
			continue;
		if (notmouse && connectedaxes[i].ismouse)
			continue;
		if (!allowshifted)
		{
		 	CString s;
		 	s.LoadString(IDS_SHIFTEDINPUT);

			if (connectedaxes[i].name.Find(s)!=-1)
				continue;
		}

		if (!allownormal)
		{
		 	CString s;
		 	s.LoadString(IDS_SHIFTEDINPUT);

			if (connectedaxes[i].name.Find(s)==-1)
				continue;
		}

		if (newindex==currindex)
		{
			connectedaxes[i].assignedtask=task;
			selindex=i;
		}
		currindex++;
	}
	if (allowaliasing)
		if (allowaliasing>AA_AA_SETUP)
		{

		}
		else
		{
 			 for (Analogue::AliassingOption* ao=Analogue::Aliassing_Table[allowaliasing];ao->gamereason;ao++)
				 if (newindex==ao->aliassercomboindex)
				 {	//I have selected this option
					
					_Analogue.tune[ao->aliasedtask].aliased|=allowaliasing;
					if (paironly)
						_Analogue.tune[ao->aliasedtask+1].aliased|=allowaliasing;
					for (int i=firstaxes;i<usedaxes;i++)
						if (connectedaxes[i].assignedtask==task)
							connectedaxes[i].assignedtask=AU_UNASSIGNED;

				 }
				 else
				 {
					_Analogue.tune[ao->aliasedtask].aliased%=allowaliasing;
					if (paironly)
						_Analogue.tune[ao->aliasedtask+1].aliased%=allowaliasing;

				 }
		}
	return selindex;

}

void	SController::ChangedAxesValue(int newind,AxisUses task)
{
	for (int i=firstaxes;i<usedaxes;i++)
	{
		if (connectedaxes[i].assignedtask==task)
			connectedaxes[i].assignedtask=AU_UNASSIGNED;
	}
	if (newind>=0)
		connectedaxes[newind].assignedtask=task;
}

bool	SController::SetAxesCombo(CRCombo& m_ID,AxisUses task,bool xonly,bool analogonly,bool paironly,bool notstick, bool notmouse, bool allowshifted, bool allownormal,AllowAliasing allowaliassing)
{
	CRCombo* combo=GETDLGITEM(ID);
	combo->Clear();
	int currindex=1;
	int selindex=0;
	bool	rv=false;

	{
		if (task==AU_COCKPIT || task==AU_GUNH)											//AMM 28Jul00
			combo->AddString(RESSTRING(NONE_WORD));							//AMM 28Jul00
		else
			combo->AddString(RESSTRING(KEYBOARD));

		for (int i=firstaxes;i<usedaxes;i++)
		{
			if (!connecteddevices[connectedaxes[i].devnum].enablepoll)
				continue;
			if (xonly && !connectedaxes[i].xtype)
				continue;
			if (analogonly && connectedaxes[i].ishat)
				continue;
			if (paironly && !connectedaxes[i].firstofpair)
				continue;
			if (notstick && !connectedaxes[i].ismouse)
				continue;
			if (notmouse && connectedaxes[i].ismouse)
				continue;
			if (!allowshifted)
			{
		 		CString s;
		 		s.LoadString(IDS_SHIFTEDINPUT);

				if (connectedaxes[i].name.Find(s)!=-1)
					continue;
			}
			if (!allownormal)
			{
		 		CString s;
		 		s.LoadString(IDS_SHIFTEDINPUT);

				if (connectedaxes[i].name.Find(s)==-1)
					continue;
			}

			if (connectedaxes[i].assignedtask!=AU_UNASSIGNED)
			{
				if (connectedaxes[i].assignedtask!=task	  )
				{
					continue;
				}
				else
				{
					rv=connectedaxes[i].ishat;
					selindex=currindex;
				}
			}
			if (paironly && !connectedaxes[i].ishat)
				combo->AddString(CSprintf("%s %s & %s",
										connecteddevices[connectedaxes[i].devnum].name,
										connectedaxes[i].name,connectedaxes[i+1].name));
			else
				combo->AddString(CSprintf("%s %s",
										connecteddevices[connectedaxes[i].devnum].name,
										connectedaxes[i].name			));
			currindex++;
		}
		if (allowaliassing && allowaliassing<AA_AA_SETUP)
		{	//this axis can alias on another axis
				 for (Analogue::AliassingOption* ao=Analogue::Aliassing_Table[allowaliassing];ao->gamereason;ao++)
				 {
					 if (ao->aliassedtaskgotanalogueinput)
					 {
						 CString aliassedname=ao->aliassedcombo->GetCaption();
						 ao->aliassercomboindex=currindex;
						 combo->AddString(CSprintf("%s: %s",LoadResString(ao->aliaseed_IDS),aliassedname));
						 if ( _Analogue.tune[ao->aliasedtask].aliased[allowaliassing])
							selindex=currindex;
						 currindex++;
					 }
					 else
						 ao->aliassercomboindex=-1;

				 }
			}

			{	 //this axis can be aliassed. need to find this device in the list
				 Analogue::AliassingOption* ao;
				 for (ao=Analogue::Aliassing_List;ao->aliasedtask!=AU_ILLEGAL;ao++)
					 if(ao->gamereason && ao->aliasedtask==task)
					 {	//there is another input that can alias this input
						 if (selindex>0)
						 {	//this input is analogue enabled, so other input can be aliassed to this
							 ao->aliassedtaskgotanalogueinput=true;
						 }
						 else
						 {	 //this input is keyboard only, so other input can't alias this one!
							 ao->aliassedtaskgotanalogueinput=false;
							 _Analogue.tune[task].aliased%=ao->gamereason;
							 if (paironly)
								 _Analogue.tune[task+1].aliased%=ao->gamereason;
						 }
						 ao->aliassedcombo=combo;
					 }
			}
	}
	combo->SetIndex(selindex);
	return rv;
}
/////////////////////////////////////////////////////////////////////////////
// SController message handlers
void SController::OnClickedCalib() 
{
	// TODO: Add your control notification handler code here
	CRCombo* combo=GETDLGITEM(IDC_DEVLIST);
	int	index=combo->GetIndex();
	LPDIRECTINPUTDEVICE dev;
	LPDIRECTINPUT	DIdev;
	DirectInputCreate(AfxGetInstanceHandle( ),DIRECTINPUT_VERSION,&DIdev,NULL);
	if (connecteddevices[index].devid.Data1)
	{
		if (SUCCEEDED(DIdev->CreateDevice(connecteddevices[index].devid,&dev,NULL)))
		{
			dev->RunControlPanel(Master_3d.winst,0);
//OLD_CODE			dev->RunControlPanel(m_hWnd,0);
			dev->Release();
		}
		else
			DIdev->RunControlPanel(Master_3d.winst,0);
//OLD_CODE			DIdev->RunControlPanel(m_hWnd,0);
	}
	else
		DIdev->RunControlPanel(Master_3d.winst,0);
//OLD_CODE		DIdev->RunControlPanel(m_hWnd,0);
	DIdev->Release();
	SetAxisConfig(false,false);
	BuildEnumerationTables();
	GetAxisConfig();
	RemakeDisplay();
}

//void SController::OnClickedCalib2() 
//{
//	// TODO: Add your control notification handler code here
//	_Analogue.DIdev->RunControlPanel(m_hWnd,0);
//	SetAxisConfig();
//	BuildEnumerationTables();
//	GetAxisConfig();
//	RemakeDisplay();
//
//}

void SController::OnTextChangedDevlist(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	SetAxisConfig(false,false);
	BuildEnumerationTables();
	GetAxisConfig();
	RemakeDisplay();

	
}

void SController::OnClickedSelff() 
{
	// TODO: Add your control notification handler code here
	CRCombo* combo=GETDLGITEM(IDC_DEVLIST);
	int	index=combo->GetIndex();
	CRButton* button=GETDLGITEM(IDC_SELFF);
	if (connecteddevices[index].supportsff && connecteddevices[index].enablepoll)
	{
		connecteddevices[index].useff=(connecteddevices[index].useff==false);
//		connecteddevices[index].enablepoll=true;
//		button->SetPressed(true);
	}	
	else
		button->SetPressed(false);

	int i;



}

void SController::OnClickedSelpoll() 
{
	// TODO: Add your control notification handler code here
	CRCombo* combo=GETDLGITEM(IDC_DEVLIST);
	int	index=combo->GetIndex();
	CRButton* button=GETDLGITEM(IDC_SELPOLL);
	bool	nowset=button->GetPressed()!=0;
	int i;
	for (i=firstaxes;i<usedaxes;i++)
		breakif(connectedaxes[i].devnum==index && connectedaxes[i].assignedtask!=AU_UNASSIGNED);
	int altind=-1;
	if (nowset && i==usedaxes)
		if (index==0)
			altind=joystickalias;
		elseif (index==1)
			altind=mousealias;
		elseif (index==joystickalias)
			altind=0;
		elseif (index==mousealias)
			altind=1;
	if (altind!=-1)
	{	//disable the other version of this stick...
		connecteddevices[altind].enablepoll=false;
		connecteddevices[index].useff=connecteddevices[altind].useff;

		connecteddevices[altind].useff=false;

		GETDLGITEM(IDC_SELFF)->SetPressed(connecteddevices[index].useff);

		for (int i=firstaxes;i<usedaxes;i++)
			if (connectedaxes[i].devnum==altind && connectedaxes[i].assignedtask!=AU_UNASSIGNED)
			{
				int j;
				for (j=firstaxes;j<usedaxes;j++)
					breakif (	connectedaxes[j].devnum==index 
						&&	connectedaxes[j].axisnum==connectedaxes[i].axisnum
						&&	connectedaxes[j].ishat==connectedaxes[i].ishat	);
				
				connectedaxes[j].assignedtask=connectedaxes[i].assignedtask;
				connectedaxes[i].assignedtask=AU_UNASSIGNED;
			}

	}
	if (i<usedaxes)
		button->SetPressed(true);
	else
	{
		connecteddevices[index].enablepoll=nowset;			
		RemakeAxes();
	}
}


void SController::OnTextChangedMousedev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_MOUSEDEV,AU_UI_X,true,false,true,false,false,false,true);
	if (newaxis>=0 && !connectedaxes[newaxis].ishat)
		ChangedAxesValue(newaxis+1,AU_UI_Y);
	else
		ChangedAxesValue(-1,AU_UI_Y);

	RemakeAxes();
	SetEnabled(newaxis);
}

void	SController::SetEnabled(int newaxis)
{
	CRCombo* combo=GETDLGITEM(IDC_DEVLIST);
	int	index=combo->GetIndex();
	if (newaxis>=0 && index==connectedaxes[newaxis].devnum && !connecteddevices[index].enablepoll)
	{
		CRButton* button=GETDLGITEM(IDC_SELPOLL);
		button->SetPressed(true);
		connecteddevices[index].enablepoll=true;
	}
}
void SController::OnTextChangedPandev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_PANDEV,AU_VIEWH,false,false,true,false,false,true,true);
	if (newaxis>=0)
	{
		if (connectedaxes[newaxis].firstofpair)
			if (connectedaxes[newaxis].ishat)
				ChangedAxesValue(-1,AU_VIEWP);
			else
				ChangedAxesValue(newaxis+1,AU_VIEWP);
	}
	else
	{
		ChangedAxesValue(-1,AU_VIEWP);
	}
	RemakeAxes();
	SetEnabled(newaxis);
}


void SController::OnTextChangedRuddev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_RUDDEV,AU_RUDDER,false,true,false,false,true,false,true);	
	SetEnabled(newaxis);
	RemakeAxes();
}


void SController::OnTextChangedStickdev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_STICKDEV,AU_AILERON,true,true,true,false,true,false,true);
	ChangedAxesValue(newaxis+1,AU_ELEVATOR);
	SetEnabled(newaxis);
	RemakeAxes();
}

void SController::OnTextChangedThrotdev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_THROTDEV,AU_THROTTLE,false,true,false,false,false,false,true);	
	SetEnabled(newaxis);
	RemakeAxes();
}

void SController::OnTextChangedZoomdev(LPCTSTR text, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_ZOOMDEV,AU_VIEWZ,false,true,false,true,false,false,true);	
	SetEnabled(newaxis);
	RemakeAxes();
}

void SController::OnTimer(UINT nIDEvent) 
{
	static ULong con_time=timeGetTime()+3000;

	if (nIDEvent==ControlsPref_Timer && _DPlay.PlayerCreated)
	{
		_DPlay.UIUpdateMainSheet();
	}

// check for devices every second in case a device is removed via calibration, 
// unplugged or whatever

		if (!GotSameDevices())
		{
			SetAxisConfig(false,false);
			BuildEnumerationTables();
			GetAxisConfig();
			RemakeDisplay();
		}

	CDialog::OnTimer(nIDEvent);
}

void SController::OnDestroy() 
{
	KillTimer(ControlsPref_Timer);

	CDialog::OnDestroy();

}



static void SetDZCombo(SController::CRCombo* combo,int level,int midlevel)
{
	int index=0;
	if (level>=midlevel)
	{
		if (level==midlevel)
			index=1;
		else
			index=2;
	}
		combo->RESCOMBO(SMALL,3)->SetIndex(index);

}
BOOL SController::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//Moved this form DoDataExchange. Using both functions is dicey and confusing...
	CREdtBt*	calib=GETDLGITEM(IDC_CALIB);
	calib->SetCaption(RESSTRING(CALIBRATE));
	BuildEnumerationTables();
	GetAxisConfig();
	RemakeDisplay();


	CRCombo* combo;
	
	if (_DPlay.PlayerCreated)									//AMM 12Jul99
	{															//AMM 12Jul99
		_DPlay.SendStatusChange(DPlay::CPS_PREFS);
	}															//AMM 12Jul99

	ControlsPref_Timer=SetTimer(DPlay::TIMER_PREFSCONTROLS,0,NULL);	//AMM 12Jul99


//Rudder
	SetDZCombo(GETDLGITEM(IDC_CBO_RUDDZ),_Analogue.tune[AU_RUDDER].deadzones,DZ_MEDIUM);
	combo=GETDLGITEM(IDC_CBO_RUDMODE);
	int index = _Analogue.tune[AU_RUDDER].mode;
	combo->RESCOMBO(ARCADE,2)->SetIndex(index);




//Stick
	SetDZCombo(GETDLGITEM(IDC_CBO_STICKDZ),_Analogue.tune[AU_AILERON].deadzones,DZ_MEDIUM);
	combo=GETDLGITEM(IDC_CBO_STICKMODE);
	index = _Analogue.tune[AU_AILERON].mode;
	combo->RESCOMBO(ARCADE,2)->SetIndex(index);
//Throttle
	SetDZCombo(GETDLGITEM(IDC_CBO_THROTDZ),_Analogue.tune[AU_THROTTLE].deadzones,DZ_MEDIUM);

//Throttle2
	SetDZCombo(GETDLGITEM(IDC_CBO_THROT2DZ),_Analogue.tune[AU_THROTTLE2].deadzones,DZ_MEDIUM);
 //PropPitch
	SetDZCombo(GETDLGITEM(IDC_CBO_PPDZ),_Analogue.tune[AU_PROPPITCH].deadzones,DZ_MEDIUM);
//PropPitch2
	SetDZCombo(GETDLGITEM(IDC_CBO_PP2DZ),_Analogue.tune[AU_PROPPITCH2].deadzones,DZ_MEDIUM);

//pan
	SetDZCombo(GETDLGITEM(IDC_CBO_PANDZ),_Analogue.tune[AU_VIEWP].deadzones,DZ_MEDIUM);
 	combo=GETDLGITEM(IDC_CBO_PANMODE);
	index = _Analogue.tune[AU_VIEWP].mode - AM_LOW;
	combo->RESCOMBO(LOWMODE,2)->SetIndex(index);
//zoom
	combo=GETDLGITEM(IDC_CBO_ZOOMMODE);
	index = _Analogue.tune[AU_VIEWZ].mode - AM_LOW;
	combo->RESCOMBO(LOWZOOM,3)->SetIndex(index);
//cpit
	combo=GETDLGITEM(IDC_CBO_COCKPITMODE);
	index = _Analogue.tune[AU_COCKPIT].mode - AM_LOW;
	combo->RESCOMBO(LOWZOOM,3)->SetIndex(index);

//gunner
	SetDZCombo(GETDLGITEM(IDC_CBO_GUNNERDZ),_Analogue.tune[AU_GUNP].deadzones,DZ_MEDIUM);
	combo=GETDLGITEM(IDC_CBO_GUNNERMODE);
	index = _Analogue.tune[AU_GUNP].mode - AM_LOW;
	combo->RESCOMBO(LOWGUNNER,3)->AddString(RESSTRING(MOVEORDERED));
	combo->SetIndex(index);

	CRButton* button=GETDLGITEM(IDC_FLIP_GUNNER);
	button->SetPressed(_Analogue.tune[AU_GUNP].aliased[AA_TT_REVERSE]);
	button=GETDLGITEM(IDC_FLIP_PAN);
	button->SetPressed(_Analogue.tune[AU_VIEWP].aliased[AA_TT_REVERSE]);
	button=GETDLGITEM(IDC_FLIP_COCKPIT);
	button->SetPressed(_Analogue.tune[AU_UI_Y].aliased[AA_TT_REVERSE]);
 
	button=GETDLGITEM(IDC_FLIP_PANHORI);
	button->SetPressed(_Analogue.tune[AU_VIEWH].aliased[AA_TT_REVERSE]);


//Force Feedback
	combo=GETDLGITEM(IDC_CBO_FF_GUNRATTLE);								//CSB 17Aug00
	index = _Analogue.FF_gun;											//CSB 17Aug00
	combo->RESCOMBO(OBLMHT_OFF,6)->SetIndex(index);						//CSB 17Aug00
	combo=GETDLGITEM(IDC_CBO_FF_BUFF);									//CSB 17Aug00
	index = _Analogue.FF_buffet;										//CSB 17Aug00
	combo->RESCOMBO(OBLMHT_OFF,6)->SetIndex(index);						//CSB 17Aug00
	combo=GETDLGITEM(IDC_CBO_FF_AERO);									//CSB 17Aug00
	index = _Analogue.FF_aerodynamic;									//CSB 17Aug00
	combo->RESCOMBO(OBLMHT_OFF,6)->SetIndex(index);						//CSB 17Aug00
	combo=GETDLGITEM(IDC_CBO_FF_AIRFRAME);								//CSB 17Aug00
	index = _Analogue.FF_airframe;										//CSB 17Aug00
	combo->RESCOMBO(OBLMHT_OFF,6)->SetIndex(index);						//CSB 17Aug00



	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void SController::OnTextChangedCboStickdz(LPCTSTR Caption, short index) 
{

	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_AILERON].deadzones = DZ_SMALL;
			_Analogue.tune[AU_ELEVATOR].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_AILERON].deadzones = DZ_MEDIUM;
			_Analogue.tune[AU_ELEVATOR].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_AILERON].deadzones = DZ_LARGE;
			_Analogue.tune[AU_ELEVATOR].deadzones = DZ_LARGE;
			break;
		}
	}
	
}

void SController::OnTextChangedCboStickdmode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_AILERON].mode = index;
	_Analogue.tune[AU_ELEVATOR].mode = index;
	
}

void SController::OnTextChangedThrotdev2(LPCTSTR Caption, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_THROTDEV_2,AU_THROTTLE2,false,true,false,false,false,false,true,AA_THROTTLE2_PROP1);
	SetEnabled(newaxis);
	RemakeAxes();
	
}

void SController::OnTextChangedProppitch2(LPCTSTR Caption, short index) 
{
	// TODO: Add your control notification handler code here

	
	int newaxis=ChangedAxesCombo(m_IDC_PROPPITCH2,AU_PROPPITCH2,false,true,false,false,false,false,true);
	SetEnabled(newaxis);
	RemakeAxes();
}

void SController::OnTextChangedProppitch(LPCTSTR Caption, short index) 
{
	// TODO: Add your control notification handler code here
	
	int newaxis=ChangedAxesCombo(m_IDC_PROPPITCH,AU_PROPPITCH,false,true,false,false,false,false,true);
	SetEnabled(newaxis);
	RemakeAxes();
}

void SController::OnTextChangedInteractivecockpit(LPCTSTR Caption, short index) 
{
	// TODO: Add your control notification handler code here
	int newaxis=ChangedAxesCombo(m_IDC_INTERACTIVECPIT,AU_COCKPIT,true,false,true,true,false,true,true,AA_UI_PAN);
	ChangedAxesValue(newaxis+1,(AxisUses)(AU_COCKPIT+1));
	SetEnabled(newaxis);
	RemakeAxes();
	
}

void SController::OnTextChangedGunner(LPCTSTR Caption, short index) 
{
	// TODO: Add your control notification handler code here
	
	int newaxis=ChangedAxesCombo(m_IDC_GUNNER,AU_GUNH,false,true,true,false,false,true,true,AA_GUNNER_JOYVIEW);
	ChangedAxesValue(newaxis+1,AU_GUNP);
	SetEnabled(newaxis);
	RemakeAxes();

}

void SController::OnTextChangedCboZoommode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_VIEWZ].mode = index+AM_LOW;
	
}



void SController::OnTextChangedCboThrotdz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_THROTTLE].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_THROTTLE].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_THROTTLE].deadzones = DZ_LARGE;
			break;
		}
	}
	
}


void SController::OnTextChangedCboThrot2dz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_THROTTLE2].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_THROTTLE2].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_THROTTLE2].deadzones = DZ_LARGE;
			break;
		}
	}
	
}

void SController::OnTextChangedCboRudmode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_RUDDER].mode = index;
	
}

void SController::OnTextChangedCboRuddz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_RUDDER].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_RUDDER].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_RUDDER].deadzones = DZ_LARGE;
			break;
		}
	}
	
}


void SController::OnTextChangedCboPpdz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_PROPPITCH].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_PROPPITCH].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_PROPPITCH].deadzones = DZ_LARGE;
			break;
		}
	}
	
}


void SController::OnTextChangedCboPp2dz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_PROPPITCH2].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_PROPPITCH2].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_PROPPITCH2].deadzones = DZ_LARGE;
			break;
		}
	}
	
}

void SController::OnTextChangedCboPanmode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_VIEWP].mode = index+AM_LOW;
	_Analogue.tune[AU_VIEWH].mode = index+AM_LOW;
	
}

void SController::OnTextChangedCboPandz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_VIEWP].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_VIEWP].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_VIEWP].deadzones = DZ_LARGE;
			break;
		}
	}
	_Analogue.tune[AU_VIEWH].deadzones = _Analogue.tune[AU_VIEWP].deadzones;
}

void SController::OnTextChangedCboGunnermode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_GUNP].mode = index+AM_LOW;
	_Analogue.tune[AU_GUNH].mode = index+AM_LOW;
	
}

void SController::OnTextChangedCboGunnerdz(LPCTSTR Caption, short index) 
{
	switch (index)
	{
		case 0:	
		{
			_Analogue.tune[AU_GUNP].deadzones = DZ_SMALL;
			break;
		}
		case 1:	
		{
			_Analogue.tune[AU_GUNP].deadzones = DZ_MEDIUM;
			break;
		}
		case 2:	
		{
			_Analogue.tune[AU_GUNP].deadzones = DZ_LARGE;
			break;
		}
	}
	_Analogue.tune[AU_GUNH].deadzones = _Analogue.tune[AU_GUNP].deadzones;
	
}

void SController::OnTextChangedCboCockpitmode(LPCTSTR Caption, short index) 
{
	_Analogue.tune[AU_UI_X].mode = AM_LOW+index;
	_Analogue.tune[AU_UI_Y].mode = AM_LOW+index;

}


void SController::OnTextChangedCboFfGunrattle(LPCTSTR Caption, short index) 
{
	_Analogue.FF_gun = index;											//CSB 17Aug00
}

void SController::OnTextChangedCboFfBuff(LPCTSTR Caption, short index) 
{
	_Analogue.FF_buffet = index;										//CSB 17Aug00
}

void SController::OnTextChangedCboFfAirframe(LPCTSTR Caption, short index) 
{
	_Analogue.FF_airframe = index;										//CSB 17Aug00
}

void SController::OnTextChangedCboFfAero(LPCTSTR Caption, short index) 
{
	_Analogue.FF_aerodynamic = index;									//CSB 17Aug00
}

void SController::OnClickedFlipCockpit() 
{
	   _Analogue.tune[AU_UI_Y].aliased^=AA_TT_REVERSE;
	
}

void SController::OnClickedFlipGunner() 
{
	   _Analogue.tune[AU_GUNP].aliased^=AA_TT_REVERSE;	//reverse = false;
	
}

void SController::OnClickedFlipPan() 
{
	   _Analogue.tune[AU_VIEWP].aliased^=AA_TT_REVERSE;	//reverse = false;
	
}

void SController::OnClickedFlipPanhori() 
{
	   _Analogue.tune[AU_VIEWH].aliased^=AA_TT_REVERSE;	//reverse = false;
	
}
