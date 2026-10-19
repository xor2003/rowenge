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

// RowanDialog: // DirRsult.cpp : implementation file
//

#include "stdafx.h"
#include "MIGView.h"
#include "bob.h"
#include "DirRsult.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// DirectiveResults dialog


RDialog*	DirectiveResults::Make(LWDirectivesResults* dirres)
{
	return
		MakeTopDialog(::Place(POSN_CENTRE,POSN_CENTRE),
			DialBox(FIL_D_BLANK,new DirectiveResults(dirres))	//MS 15Nov100
			);
}


DirectiveResults::DirectiveResults(LWDirectivesResults* dirres, CWnd* pParent /*=NULL*/)
	: RowanDialog(DirectiveResults::IDD, pParent)
{
	//{{AFX_DATA_INIT(DirectiveResults)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	dr = dirres;
}
DirectiveResults::~DirectiveResults()
{
	if (dr)
		delete dr;
}

void DirectiveResults::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DirectiveResults)
	DDX_Control(pDX, IDC_RLISTBOXCTRL1, m_IDC_RLISTBOXCTRL1);
	//}}AFX_DATA_MAP
	GETDLGITEM(IDC_RLISTBOXCTRL1)->AddString("",0);
}


BEGIN_MESSAGE_MAP(DirectiveResults, CDialog)
	//{{AFX_MSG_MAP(DirectiveResults)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DirectiveResults message handlers


void DirectiveResults::Refresh()
{
	CRListBox* rlistbox;
	rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);

	int i = 0;
	while (dr->dirresults[i].targets[0] != UID_NULL)
	{

		int fighters =  (	dr->dirresults[i].mission.retfighters + 
							dr->dirresults[i].mission.detfighters + 
							dr->dirresults[i].mission.attfighters  
						);
		//positive means gruppe and negative means staffel
		if (fighters < 0)						
			fighters = -12* fighters;		
		else
			fighters = 36 * fighters;
		int bombers = dr->dirresults[i].mission.bombers;
		if 	(bombers < 0)
			bombers = -10 * bombers;
		rlistbox->AddString(CSprintf("%i", bombers),0);
		rlistbox->AddString(CSprintf("%i", fighters),2);

		rlistbox->AddString(RESLIST(AM_PATROL,dr->dirresults[i].mission.bomberattackmethod),1);

		rlistbox->AddString(GetTargName(dr->dirresults[i].targets[0]),3);
		int j = 1;
		while (dr->dirresults[i].targets[j] != UID_NULL)
		{
			rlistbox->AddString("",0);
			rlistbox->AddString("",2);
			rlistbox->AddString("",1);
			rlistbox->AddString(GetTargName(dr->dirresults[i].targets[j]),3);

			j++;
		}

		i++;
	}

}
BOOL DirectiveResults::OnInitDialog() 
{
	CDialog::OnInitDialog();
		CRListBox* rlistbox;
	rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);
	rlistbox->Clear();

		//rlistbox->AddColumn(40);
		//rlistbox->AddColumn(80);										  //RDH 02/06/99
		//rlistbox->AddColumn(50);										  //RDH 02/06/99
		//rlistbox->AddColumn(TARGETFIELDWIDTH);										  //RDH 02/06/99
		rlistbox->AddString(RESSTRING(SIZE),0);
		rlistbox->AddString(RESSTRING(TYPE),1);
		rlistbox->AddString(RESSTRING(ESCORT),2);
		rlistbox->AddString(RESSTRING(TARGET),3);

		Refresh();


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DirectiveResults::OnCancel() 
{
#ifndef	BOB_DEMO_VER
	if (m_pView->m_currentpage==0)
		MiscToolBar().OpenDirectivetoggle(dr);

#endif

	dr = NULL;
	
	CDialog::OnCancel();
}
void DirectiveResults::OnOK() 
{
	LWDirectivesResults::MakeLWPackages(dr, true);
	m_pView->m_mapdlg.Invalidate();
	MMC.directives.lw.current =	dr->tempdir;

	if (Save_Data.minsbetweensavegame)						//RJS 26Oct00
		CFiling::SaveTimedGame();							//RJS 26Oct00


	CDialog::OnOK();
}

 typedef	Profile::AttackMethod	AttackMethod;
 MAKEFIELD(AttackMethod,Profile::AM_FIELD_MIN,Profile::AM_FIELD_MAX);


void LWDirectivesResults::MakeLWPackages(LWDirectivesResults* dr, bool manual)
{
	Todays_Packages.squelchfuelwarning=true;
	bool	anyofpacksbad=false;
	int i = 0;
	int missioncnt = 0;
	int deltamissiontime;
	if 	(dr->tempdir.arrivalminsseperation <= 2)
		deltamissiontime =  HR02 + Math_Lib.rnd(HR02);
	else
		deltamissiontime =  Math_Lib.rnd(HR02);


	int missiontime =  deltamissiontime + MMC.currperiodtime;
	int missiontimeinc = 0;
	while (dr->dirresults[missioncnt].targets[0] != UID_NULL)
		 missioncnt++;
	if (		(dr->tempdir.arrivalminsseperation <= 2)
			||	(missioncnt < 2)
		)
	{
		missiontimeinc = 0;
	}else
	{
		missiontimeinc = (HR04 - deltamissiontime)/(missioncnt - 1);
	}
	i =0;
	while (dr->dirresults[i].targets[0] != UID_NULL)
	{
		int packnum;
		//the following are filled in:
		//dr->dirresults[k].mission.escortmethods
		bool gruppe = true;
		int fighters =  (	dr->dirresults[i].mission.retfighters + 
							dr->dirresults[i].mission.detfighters + 
							dr->dirresults[i].mission.attfighters  
						);

		if (fighters < 0)
			gruppe = false;
		packnum=Todays_Packages_NewPackage(dr->dirresults[i].targets[0], 
											dr->dirresults[i].mission,
											&dr->dirresults[i].targets[1],
											gruppe,
											dr->dirresults[i].usesquadsavail);


		if (packnum!=-1)
		{
			if (!manual)
			{
				int i = 0,j;
				TargetNoDetail*	targp=Node_Data[dr->dirresults[i].targets[0]];

				while	(			(i <TargetGroup::MAXDOGLEGS)
								&&	(TargetGroupPtr(targp)->doglegs[i])
						)
					i++;
				j = Math_Lib.rnd(i);
				if (TargetGroupPtr(targp)->doglegs[j] != UID_NULL)
					Todays_Packages[packnum].doglegloc=TargetGroupPtr(targp)->doglegs[j];


			}
			Todays_Packages[packnum].CalcRoutePositionsAndTime(missiontime);
			if (Todays_Packages[packnum].cancelsquad)
				anyofpacksbad=true;
										
			if 	(dr->tempdir.arrivalminsseperation >= 15)		//RDH 9Aug00
				missiontime += missiontimeinc - MIN30 + Math_Lib.rnd(HR01);
			if 	((missiontime - MMC.currperiodtime) > 	HR04)
				missiontime = MMC.currperiodtime + 	HR04;
		}
		i++;
	}

	Todays_Packages.squelchfuelwarning=false;
	if (anyofpacksbad &&  Todays_Packages.localplayer!=NAT_RAF)
		MainToolBar().OpenMissionfolder(true);
	//temp to set up raf
}
Directives::LW::MissTypes  LWDirectivesResults::LWMissType()
{
	Profile& pk = Todays_Packages.pack[Todays_Packages.hipack];
	Directives::LW::MissTypes misstype;
	if (pk.attackmethod  ==  Profile::AM_DECOYPATROL)
		misstype = Directives::LW::PATROL;
	else if (pk.attackmethod  ==  Profile::AM_RECON)
		misstype = Directives::LW::RECON;
	else 
		misstype = Directives::LW::ATTACK;
	return (misstype);
}
