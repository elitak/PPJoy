/***************************************************************************
 *   PPJoy Virtual Joystick for Microsoft Windows                          *
 *   Copyright (C) 2011 Deon van der Westhuysen                            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


// TimingWizard.cpp : implementation file
//

#include "stdafx.h"
#include "ppjoycpl.h"
#include "TimingWizard.h"

#include "JoyBus.h"
#include "ConfigMan.h"
#include "Mapping.h"

#include "ResourceStrings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard

IMPLEMENT_DYNAMIC(CTimingWizard, CPropertySheet)

CTimingWizard::CTimingWizard()
	:CPropertySheet()
{
 DefaultTiming= NULL;
 DriverTiming= NULL;
 DeviceTiming= NULL;

 DelTimingPage.AddToWizard(this);
 FMSPage.AddToWizard(this);
 GenesisPage.AddToWizard(this);
 LPTswitchPage.AddToWizard(this);
 PSXPage.AddToWizard(this);
 SNESPage.AddToWizard(this);
 StartPage.AddToWizard(this);

 SetWizardMode();
 SetActivePage(&StartPage);
}

//CTimingWizard::CTimingWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
//	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
//{
//}
//
//CTimingWizard::CTimingWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
//	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
//{
//}

CTimingWizard::~CTimingWizard()
{
 if (DefaultTiming)
  free(DefaultTiming);
 if (DriverTiming)
  free(DriverTiming);
 if (DeviceTiming)
  free(DeviceTiming);
}


BEGIN_MESSAGE_MAP(CTimingWizard, CPropertySheet)
	//{{AFX_MSG_MAP(CTimingWizard)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_FMSPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_FMSPage, CPropertyPage)

CTimingWizard_FMSPage::CTimingWizard_FMSPage() : CPropertyPage(CTimingWizard_FMSPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_FMSPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTimingWizard_FMSPage::~CTimingWizard_FMSPage()
{
}

void CTimingWizard_FMSPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_FMSPage)
	DDX_Control(pDX, IDC_RESETDEFAULT, m_ResetButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_SYNCEDIT, m_SyncPulseEdit);
	DDX_Control(pDX, IDC_MINPULSEEDIT, m_MinPulseEdit);
	DDX_Control(pDX, IDC_MAXPULSEEDIT, m_MaxPulseEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_FMSPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_FMSPage)
	ON_BN_CLICKED(IDC_APPLY, OnApplySettings)
	ON_BN_CLICKED(IDC_RESETDEFAULT, OnResetdefault)
	ON_EN_CHANGE(IDC_MAXPULSEEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_MINPULSEEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_SYNCEDIT, OnEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_FMSPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_GenesisPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_GenesisPage, CPropertyPage)

CTimingWizard_GenesisPage::CTimingWizard_GenesisPage() : CPropertyPage(CTimingWizard_GenesisPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_GenesisPage)
	//}}AFX_DATA_INIT
}

CTimingWizard_GenesisPage::~CTimingWizard_GenesisPage()
{
}

void CTimingWizard_GenesisPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_GenesisPage)
	DDX_Control(pDX, IDC_RESETDEFAULT, m_ResetButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_BIT6DELAYEDIT, m_Bit6DelayEdit);
	DDX_Control(pDX, IDC_BITDELAYEDIT, m_BitDelayEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_GenesisPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_GenesisPage)
	ON_BN_CLICKED(IDC_APPLY, OnApplySettings)
	ON_BN_CLICKED(IDC_RESETDEFAULT, OnResetdefault)
	ON_EN_CHANGE(IDC_BIT6DELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_BITDELAYEDIT, OnEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_GenesisPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_LPTswitchPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_LPTswitchPage, CPropertyPage)

CTimingWizard_LPTswitchPage::CTimingWizard_LPTswitchPage() : CPropertyPage(CTimingWizard_LPTswitchPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_LPTswitchPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTimingWizard_LPTswitchPage::~CTimingWizard_LPTswitchPage()
{
}

void CTimingWizard_LPTswitchPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_LPTswitchPage)
	DDX_Control(pDX, IDC_RESETDEFAULT, m_ResetButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_ROWDELAYEDIT, m_RowDelayEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_LPTswitchPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_LPTswitchPage)
	ON_BN_CLICKED(IDC_APPLY, OnApplySettings)
	ON_BN_CLICKED(IDC_RESETDEFAULT, OnResetdefault)
	ON_EN_CHANGE(IDC_ROWDELAYEDIT, OnEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_LPTswitchPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_PSXPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_PSXPage, CPropertyPage)

CTimingWizard_PSXPage::CTimingWizard_PSXPage() : CPropertyPage(CTimingWizard_PSXPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_PSXPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTimingWizard_PSXPage::~CTimingWizard_PSXPage()
{
}

void CTimingWizard_PSXPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_PSXPage)
	DDX_Control(pDX, IDC_RESETDEFAULT, m_ResetButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_TAILDELAYEDIT, m_TailDelayEdit);
	DDX_Control(pDX, IDC_SELDELAYEDIT, m_SelDelayEdit);
	DDX_Control(pDX, IDC_BITDELAYEDIT, m_BitDelayEdit);
	DDX_Control(pDX, IDC_ACKXDELAYEDIT, m_AckXDelayEdit);
	DDX_Control(pDX, IDC_ACK1DELAYEDIT, m_Ack1DelayEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_PSXPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_PSXPage)
	ON_BN_CLICKED(IDC_APPLY, OnApplySettings)
	ON_BN_CLICKED(IDC_RESETDEFAULT, OnResetdefault)
	ON_EN_CHANGE(IDC_BITDELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_ACK1DELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_ACKXDELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_SELDELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_TAILDELAYEDIT, OnEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_PSXPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_SNESPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_SNESPage, CPropertyPage)

CTimingWizard_SNESPage::CTimingWizard_SNESPage() : CPropertyPage(CTimingWizard_SNESPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_SNESPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTimingWizard_SNESPage::~CTimingWizard_SNESPage()
{
}

void CTimingWizard_SNESPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_SNESPage)
	DDX_Control(pDX, IDC_RESETDEFAULT, m_ResetButton);
	DDX_Control(pDX, IDC_APPLY, m_ApplyButton);
	DDX_Control(pDX, IDC_SETUPDELAYEDIT, m_SetupDelayEdit);
	DDX_Control(pDX, IDC_BITDELAYEDIT, m_BitDelayEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_SNESPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_SNESPage)
	ON_BN_CLICKED(IDC_APPLY, OnApplySettings)
	ON_BN_CLICKED(IDC_RESETDEFAULT, OnResetdefault)
	ON_EN_CHANGE(IDC_SETUPDELAYEDIT, OnEditChanged)
	ON_EN_CHANGE(IDC_BITDELAYEDIT, OnEditChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_SNESPage message handlers

void CTimingWizard_FMSPage::OnApplySettings() 
{
 TCHAR	Buffer[128*sizeof(TCHAR)];

 m_MaxPulseEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.fmsppm.MaxPulseWidth= _tstoi (Buffer);
 m_MinPulseEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.fmsppm.MinPulseWidth= _tstoi (Buffer);
 m_SyncPulseEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.fmsppm.MinSyncWidth= _tstoi (Buffer);

 if (TimingWizard->WriteNewTiming(&NewTiming))
  m_ApplyButton.EnableWindow(FALSE);
}

void CTimingWizard_FMSPage::OnResetdefault() 
{
 RestoreValues();
 OnApplySettings();	
}

void CTimingWizard_GenesisPage::OnApplySettings() 
{
 TCHAR	Buffer[128*sizeof(TCHAR)];

 m_BitDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.genesis.BitDelay= _tstoi (Buffer);
 m_Bit6DelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.genesis.Bit6Delay= _tstoi (Buffer);

 if (TimingWizard->WriteNewTiming(&NewTiming))
  m_ApplyButton.EnableWindow(FALSE);
}

void CTimingWizard_GenesisPage::OnResetdefault() 
{
 RestoreValues();
 OnApplySettings();	
}

void CTimingWizard_LPTswitchPage::OnApplySettings() 
{
 TCHAR	Buffer[128*sizeof(TCHAR)];

 m_RowDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.lptswitch.RowDelay= _tstoi (Buffer);

 if (TimingWizard->WriteNewTiming(&NewTiming))
  m_ApplyButton.EnableWindow(FALSE);
}

void CTimingWizard_LPTswitchPage::OnResetdefault() 
{
 RestoreValues();
 OnApplySettings();	
}

void CTimingWizard_PSXPage::OnApplySettings() 
{
 TCHAR	Buffer[128*sizeof(TCHAR)];

 m_Ack1DelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.psx.Ack1Delay= _tstoi (Buffer);
 m_AckXDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.psx.AckXDelay= _tstoi (Buffer);
 m_BitDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.psx.BitDelay= _tstoi (Buffer);
 m_SelDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.psx.SelDelay= _tstoi (Buffer);
 m_TailDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.psx.TailDelay= _tstoi (Buffer);

 if (TimingWizard->WriteNewTiming(&NewTiming))
  m_ApplyButton.EnableWindow(FALSE);
}

void CTimingWizard_PSXPage::OnResetdefault() 
{
 RestoreValues();
 OnApplySettings();	
}

void CTimingWizard_SNESPage::OnApplySettings() 
{
 TCHAR	Buffer[128*sizeof(TCHAR)];

 m_BitDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.snes.BitDelay= _tstoi (Buffer);
 m_SetupDelayEdit.GetWindowText (Buffer,sizeof(Buffer)/sizeof(TCHAR));
 NewTiming.snes.SetupDelay= _tstoi (Buffer);

 if (TimingWizard->WriteNewTiming(&NewTiming))
  m_ApplyButton.EnableWindow(FALSE);
}

void CTimingWizard_SNESPage::OnResetdefault() 
{
 RestoreValues();
 OnApplySettings();	
}

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_StartPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_StartPage, CPropertyPage)

CTimingWizard_StartPage::CTimingWizard_StartPage() : CPropertyPage(CTimingWizard_StartPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_StartPage)
	//}}AFX_DATA_INIT
}

CTimingWizard_StartPage::~CTimingWizard_StartPage()
{
}

void CTimingWizard_StartPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_StartPage)
	DDX_Control(pDX, IDC_HELPTEXT, m_HelpText);
	DDX_Control(pDX, IDC_TYPECAPTION, m_TypeCaption);
	DDX_Control(pDX, IDC_JOYSTICKCAPTION, m_JoystickCaption);
	DDX_Control(pDX, IDC_MODTYPETIMINGRADIO, m_ModTypeTimingRadio);
	DDX_Control(pDX, IDC_DELTYPETIMINGRADIO, m_DelTypeTimingRadio);
	DDX_Control(pDX, IDC_MODJOYTIMINGRADIO, m_ModJoyTimingRadio);
	DDX_Control(pDX, IDC_DELJOYTIMINGRADIO, m_DelJoyTimingRadio);
	DDX_Control(pDX, IDC_ADDTYPETIMINGRADIO, m_AddTypeTimingRadio);
	DDX_Control(pDX, IDC_ADDJOYTIMINGRADIO, m_AddJoyTimingRadio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_StartPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_StartPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_StartPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_DelTimingPage property page

IMPLEMENT_DYNCREATE(CTimingWizard_DelTimingPage, CPropertyPage)

CTimingWizard_DelTimingPage::CTimingWizard_DelTimingPage() : CPropertyPage(CTimingWizard_DelTimingPage::IDD)
{
	//{{AFX_DATA_INIT(CTimingWizard_DelTimingPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTimingWizard_DelTimingPage::~CTimingWizard_DelTimingPage()
{
}

void CTimingWizard_DelTimingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingWizard_DelTimingPage)
	DDX_Control(pDX, IDC_HELPTEXT, m_HelpText);
	DDX_Control(pDX, IDC_DELCAPTION, m_DelCaption);
	DDX_Control(pDX, IDC_DELETE_YES, m_YesRadio);
	DDX_Control(pDX, IDC_DELETE_NO, m_NoRadio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingWizard_DelTimingPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTimingWizard_DelTimingPage)
	ON_BN_CLICKED(IDC_DELETE_YES, OnDeleteYes)
	ON_BN_CLICKED(IDC_DELETE_NO, OnDeleteNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingWizard_DelTimingPage message handlers

void CTimingWizard_DelTimingPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_FMSPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_GenesisPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_LPTswitchPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_PSXPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_SNESPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

void CTimingWizard_StartPage::AddToWizard(CTimingWizard *Wizard)
{
 TimingWizard= Wizard;
 TimingWizard->AddPage(this);
}

int CTimingWizard::DoModal() 
{
// DWORD			Count;
// char			DeviceID[256];

 LPTNumber= pEnumData->Joysticks[JoyIndex].LPTNumber;
 JoyType= pEnumData->Joysticks[JoyIndex].JoyType;
 UnitNumber= pEnumData->Joysticks[JoyIndex].UnitNumber;
 IFTypeName= GetJoyTypeName(JoyType);
 DevTypeName= GetDevTypeName(GetDevTypeFromJoyType(JoyType));

 ReadJoystickTimings (LPTNumber,JoyType,UnitNumber,&DefaultTiming,&DriverTiming,&DeviceTiming);
 if (!DefaultTiming)
 {
  MessageBox (GetResStr1(IDS_TIM_CANNOTREAD),GetResStr2(IDS_MSG_ERROR));
  return IDCANCEL;
 }
 
 if (CPropertySheet::DoModal()==IDCANCEL)
  return IDCANCEL;

// WaitDialog.Create(WaitDialog.IDD);
//
// for (Count=0;Count<pEnumData->Count;Count++)
//  if (((JoyType==pEnumData->Joysticks[Count].JoyType)&&(ChangeTypeMapping))||
//      (((DWORD)LPTNumber==pEnumData->Joysticks[Count].LPTNumber)&&(UnitNumber==pEnumData->Joysticks[Count].UnitNumber)))
//  {
//   MakeDeviceID (DeviceID,pEnumData->Joysticks[Count].VendorID,pEnumData->Joysticks[Count].ProductID);
//   RestartDevice (DeviceID);
//  }
//
// WaitDialog.DestroyWindow();

 return IDOK;
}

BOOL CTimingWizard_StartPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 m_AddTypeTimingRadio.SetCheck(0);
 m_ModTypeTimingRadio.SetCheck(0);
 m_DelTypeTimingRadio.SetCheck(0);

 m_AddJoyTimingRadio.SetCheck(0);
 m_ModJoyTimingRadio.SetCheck(0);
 m_DelJoyTimingRadio.SetCheck(0);

 if (TimingWizard->DriverTiming)
 {
  m_AddTypeTimingRadio.EnableWindow(FALSE);
  m_ModTypeTimingRadio.EnableWindow(TRUE);
  m_DelTypeTimingRadio.EnableWindow(TRUE);
 }
 else
 {
  m_AddTypeTimingRadio.EnableWindow(TRUE);
  m_ModTypeTimingRadio.EnableWindow(FALSE);
  m_DelTypeTimingRadio.EnableWindow(FALSE);
 }

 if (TimingWizard->DeviceTiming)
 {
  m_ModJoyTimingRadio.SetCheck(1);
  m_AddJoyTimingRadio.EnableWindow(FALSE);
  m_ModJoyTimingRadio.EnableWindow(TRUE);
  m_DelJoyTimingRadio.EnableWindow(TRUE);
 }
 else
 {
  m_AddJoyTimingRadio.SetCheck(1);
  m_AddJoyTimingRadio.EnableWindow(TRUE);
  m_ModJoyTimingRadio.EnableWindow(FALSE);
  m_DelJoyTimingRadio.EnableWindow(FALSE);
 }

 return TRUE;
}

BOOL CTimingWizard_StartPage::OnSetActive() 
{
 TCHAR	Str[256*sizeof(TCHAR)];
 TCHAR	JoystickName[128*sizeof(TCHAR)];

 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 PrintJoystickShortName (JoystickName,TimingWizard->LPTNumber,TimingWizard->JoyType,TimingWizard->UnitNumber,-1);

 _stprintf (Str,GetResStr1(IDS_TIM_CHANGEIFTIMING),TimingWizard->IFTypeName,TimingWizard->DevTypeName);
 m_TypeCaption.SetWindowText(Str);

 _stprintf (Str,GetResStr1(IDS_TIM_CHANGEJOYTIMING),JoystickName);
 m_JoystickCaption.SetWindowText(Str);

 TimingWizard->SetWizardButtons (PSWIZB_NEXT);
 return TRUE;
}

LRESULT CTimingWizard_StartPage::OnWizardNext() 
{
 LRESULT	TimingPage;

 switch (TimingWizard->JoyType)
 {
  case IF_GENESISLIN:
  case IF_GENESISDPP:
  case IF_GENESISNTP:
  case IF_GENESISCC:
  case IF_GENESISSNES:
  case IF_GENESISDPP6:
			TimingPage= (LRESULT) TimingWizard->GenesisPage.IDD;
			break;

  case IF_SNESPADLIN:
  case IF_SNESPADDPP:
  case IF_POWERPADLIN:
			TimingPage= (LRESULT) TimingWizard->SNESPage.IDD;
			break;
  
  case IF_PSXPBLIB:
  case IF_PSXDPADPRO:
  case IF_PSXLINUX:
  case IF_PSXNTPADXP:
  case IF_PSXMEGATAP:
			TimingPage= (LRESULT) TimingWizard->PSXPage.IDD;
			break;
  
  case IF_LPTSWITCH:
			TimingPage= (LRESULT) TimingWizard->LPTswitchPage.IDD;
			break;

  case IF_FMSBUDDYBOX:
			TimingPage= (LRESULT) TimingWizard->FMSPage.IDD;
			break;
  
  default:	MessageBox (GetResStr1(IDS_TIM_INTERRNODIALOG));
			return (LRESULT) -1;
 }

 if (m_AddJoyTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= FALSE;
  return TimingPage;
 }

 if (m_AddTypeTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= TRUE;
  return TimingPage;
 }

 if (m_ModJoyTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= FALSE;
  return TimingPage;
 }

 if (m_ModTypeTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= TRUE;
  return TimingPage;
 }

 if (m_DelJoyTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= FALSE;
  return (LRESULT) TimingWizard->DelTimingPage.IDD;
 }

 if (m_DelTypeTimingRadio.GetCheck())
 {
  TimingWizard->ChangeTypeTiming= TRUE;
  return (LRESULT) TimingWizard->DelTimingPage.IDD;
 }

 ASSERT(1);
 return CPropertyPage::OnWizardNext();
}


BOOL CTimingWizard_DelTimingPage::OnSetActive() 
{
 TCHAR	Str[256*sizeof(TCHAR)];
 TCHAR	JoystickName[128*sizeof(TCHAR)];

 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);
 TimingWizard->SetWizardButtons (PSWIZB_BACK|(m_YesRadio.GetCheck()?PSWIZB_FINISH:PSWIZB_DISABLEDFINISH));

 PrintJoystickShortName (JoystickName,TimingWizard->LPTNumber,TimingWizard->JoyType,TimingWizard->UnitNumber,-1);

 if (TimingWizard->ChangeTypeTiming)
 {
  _stprintf (Str,GetResStr1(IDS_TIM_DELETEIFTIMING),TimingWizard->IFTypeName,TimingWizard->DevTypeName);
  m_DelCaption.SetWindowText (Str);
  _stprintf (Str,GetResStr1(IDS_TIM_DELIFTIMINGHELP),TimingWizard->IFTypeName,TimingWizard->DevTypeName);
  m_HelpText.SetWindowText (Str);
 }
 else
 {
  _stprintf (Str,GetResStr1(IDS_TIM_DELETEJOYTIM),JoystickName);
  m_DelCaption.SetWindowText (Str);
  _stprintf (Str,GetResStr1(IDS_TIM_DELJOYTIMHELP),JoystickName,TimingWizard->IFTypeName,TimingWizard->DevTypeName);
  m_HelpText.SetWindowText (Str);
 }

 return TRUE;
}

BOOL CTimingWizard_FMSPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

BOOL CTimingWizard_GenesisPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

BOOL CTimingWizard_LPTswitchPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

BOOL CTimingWizard_PSXPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

BOOL CTimingWizard_SNESPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(TimingWizard);

 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

BOOL CTimingWizard_DelTimingPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 m_NoRadio.SetCheck(1);
 m_YesRadio.SetCheck(0);

 return TRUE;
}

void CTimingWizard_DelTimingPage::OnDeleteYes() 
{
 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
}

void CTimingWizard_DelTimingPage::OnDeleteNo() 
{
 TimingWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
}

BOOL CTimingWizard_SNESPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 RestoreValues();
 m_ApplyButton.EnableWindow(FALSE);

 return TRUE;
}

BOOL CTimingWizard_PSXPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 RestoreValues();
 m_ApplyButton.EnableWindow(FALSE);

 return TRUE;
}

BOOL CTimingWizard_LPTswitchPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 RestoreValues();
 m_ApplyButton.EnableWindow(FALSE);

 return TRUE;
}

BOOL CTimingWizard_GenesisPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 RestoreValues();
 m_ApplyButton.EnableWindow(FALSE);

 return TRUE;
}

BOOL CTimingWizard_FMSPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 RestoreValues();
 m_ApplyButton.EnableWindow(FALSE);

 return TRUE;
}

PJOYTIMING CTimingWizard::GetTiming()
{
 if ((!ChangeTypeTiming)&&DeviceTiming)
  return DeviceTiming;
 if (DriverTiming)
  return DriverTiming;

 ASSERT (DefaultTiming!=NULL);

 return DefaultTiming;
}

void CTimingWizard_FMSPage::RestoreValues()
{
 TCHAR		Buffer[128*sizeof(TCHAR)];
 PJOYTIMING	Timing;

 Timing= TimingWizard->GetTiming();

 _stprintf (Buffer,_T("%d"),Timing->fmsppm.MaxPulseWidth);
 m_MaxPulseEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->fmsppm.MinPulseWidth);
 m_MinPulseEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->fmsppm.MinSyncWidth);
 m_SyncPulseEdit.SetWindowText (Buffer);

 m_ResetButton.EnableWindow (FALSE);
}

void CTimingWizard_GenesisPage::RestoreValues()
{
 TCHAR		Buffer[128*sizeof(TCHAR)];
 PJOYTIMING	Timing;

 Timing= TimingWizard->GetTiming();

 _stprintf (Buffer,_T("%d"),Timing->genesis.BitDelay);
 m_BitDelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->genesis.Bit6Delay);
 m_Bit6DelayEdit.SetWindowText (Buffer);

 m_ResetButton.EnableWindow (FALSE);
}

void CTimingWizard_LPTswitchPage::RestoreValues()
{
 TCHAR		Buffer[128*sizeof(TCHAR)];
 PJOYTIMING	Timing;

 Timing= TimingWizard->GetTiming();

 _stprintf (Buffer,_T("%d"),Timing->lptswitch.RowDelay);
 m_RowDelayEdit.SetWindowText (Buffer);

 m_ResetButton.EnableWindow (FALSE);
}

void CTimingWizard_PSXPage::RestoreValues()
{
 TCHAR		Buffer[128*sizeof(TCHAR)];
 PJOYTIMING	Timing;

 Timing= TimingWizard->GetTiming();

 _stprintf (Buffer,_T("%d"),Timing->psx.Ack1Delay);
 m_Ack1DelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->psx.AckXDelay);
 m_AckXDelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->psx.BitDelay);
 m_BitDelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->psx.SelDelay);
 m_SelDelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->psx.TailDelay);
 m_TailDelayEdit.SetWindowText (Buffer);

 m_ResetButton.EnableWindow (FALSE);
}

void CTimingWizard_SNESPage::RestoreValues()
{
 TCHAR		Buffer[128*sizeof(TCHAR)];
 PJOYTIMING	Timing;

 Timing= TimingWizard->GetTiming();

 _stprintf (Buffer,_T("%d"),Timing->snes.SetupDelay);
 m_SetupDelayEdit.SetWindowText (Buffer);
 _stprintf (Buffer,_T("%d"),Timing->snes.BitDelay);
 m_BitDelayEdit.SetWindowText (Buffer);

 m_ResetButton.EnableWindow (FALSE);
}

BOOL CTimingWizard::WriteNewTiming(PJOYTIMING Timing)
{
 int	TimingSize;
 ULONG	Count;

 switch (JoyType)
 {
  case IF_GENESISLIN:
  case IF_GENESISDPP:
  case IF_GENESISNTP:
  case IF_GENESISCC:
  case IF_GENESISSNES:
  case IF_GENESISDPP6:
		TimingSize= sizeof(TIMING_GENESIS);
		break;

  case IF_SNESPADLIN:
  case IF_SNESPADDPP:
  case IF_POWERPADLIN:
		TimingSize= sizeof(TIMING_SNES);
		break;

  case IF_PSXPBLIB:
  case IF_PSXDPADPRO:
  case IF_PSXLINUX:
  case IF_PSXNTPADXP:
  case IF_PSXMEGATAP:
		TimingSize= sizeof(TIMING_PSX);
		break;

  case IF_LPTSWITCH:
		TimingSize= sizeof(TIMING_LPTSWITCH);
		break;

  case IF_FMSBUDDYBOX:
		TimingSize= sizeof(TIMING_FMSPPM);
		break;

  default:
		return FALSE;
 }

 if (WriteJoystickTiming (LPTNumber,JoyType,UnitNumber,Timing,TimingSize,ChangeTypeTiming?TIMING_SCOPE_DRIVER:TIMING_SCOPE_DEVICE))
 {
  MessageBox (GetResStr1(IDS_TIM_CANNOTWRITE),GetResStr2(IDS_MSG_ERROR));
  return FALSE;
 }

 if (ChangeTypeTiming)
 {
  for (Count=0;Count<pEnumData->Count;Count++)
   if (pEnumData->Joysticks[Count].JoyType==JoyType)
	ActivateJoystickTiming (pEnumData->Joysticks[Count].LPTNumber,pEnumData->Joysticks[Count].JoyType,pEnumData->Joysticks[Count].UnitNumber);
 }
 else
  ActivateJoystickTiming (LPTNumber,JoyType,UnitNumber);

 return TRUE;
}

LRESULT CTimingWizard_DelTimingPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

BOOL CTimingWizard_DelTimingPage::OnWizardFinish() 
{
 TCHAR	Msg[128*sizeof(TCHAR)];
 int	rc;

 rc= TimingWizard->DeleteTiming();

 if (rc)
 {
  _stprintf (Msg,GetResStr2(IDS_TIM_CANNOTDELETE),rc);
  MessageBox (Msg,GetResStr1(IDS_MSG_ERROR));
  return FALSE;
 }
 
 return CPropertyPage::OnWizardFinish();
}

int CTimingWizard::DeleteTiming()
{
 return DeleteJoystickTiming (LPTNumber,JoyType,UnitNumber,ChangeTypeTiming?TIMING_SCOPE_DRIVER:TIMING_SCOPE_DEVICE);
}

LRESULT CTimingWizard_FMSPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

LRESULT CTimingWizard_GenesisPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

LRESULT CTimingWizard_LPTswitchPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

LRESULT CTimingWizard_PSXPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

LRESULT CTimingWizard_SNESPage::OnWizardBack() 
{
 return (LRESULT) TimingWizard->StartPage.IDD;
}

BOOL CTimingWizard_FMSPage::OnWizardFinish()
{
 OnApplySettings();

 return CPropertyPage::OnWizardFinish();
}

BOOL CTimingWizard_GenesisPage::OnWizardFinish()
{
 OnApplySettings();

 return CPropertyPage::OnWizardFinish();
}

BOOL CTimingWizard_LPTswitchPage::OnWizardFinish()
{
 OnApplySettings();

 return CPropertyPage::OnWizardFinish();
}

BOOL CTimingWizard_PSXPage::OnWizardFinish()
{
 OnApplySettings();

 return CPropertyPage::OnWizardFinish();
}

BOOL CTimingWizard_SNESPage::OnWizardFinish()
{
 OnApplySettings();

 return CPropertyPage::OnWizardFinish();
}

void CTimingWizard_FMSPage::OnEditChanged() 
{
 m_ApplyButton.EnableWindow (TRUE);
 m_ResetButton.EnableWindow (TRUE);
}

void CTimingWizard_GenesisPage::OnEditChanged() 
{
 m_ApplyButton.EnableWindow (TRUE);
 m_ResetButton.EnableWindow (TRUE);
}

void CTimingWizard_LPTswitchPage::OnEditChanged() 
{
 m_ApplyButton.EnableWindow (TRUE);
 m_ResetButton.EnableWindow (TRUE);
}

void CTimingWizard_PSXPage::OnEditChanged() 
{
 m_ApplyButton.EnableWindow (TRUE);
 m_ResetButton.EnableWindow (TRUE);
}

void CTimingWizard_SNESPage::OnEditChanged() 
{
 m_ApplyButton.EnableWindow (TRUE);
 m_ResetButton.EnableWindow (TRUE);
}
