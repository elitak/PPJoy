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


// MapWizard.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyCpl.h"
#include "MapWizard.h"
#include "Mapping.h"
#include "JoyBus.h"
#include "ScanDialog.h"

#include "ConfigMan.h"

#include "ResourceStrings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_StartPage property page

IMPLEMENT_DYNCREATE(CMapWizard_StartPage, CPropertyPage)

CMapWizard_StartPage::CMapWizard_StartPage() : CPropertyPage(CMapWizard_StartPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_StartPage)
	//}}AFX_DATA_INIT
}

CMapWizard_StartPage::~CMapWizard_StartPage()
{
}

BOOL CMapWizard_StartPage::OnSetActive() 
{
 TCHAR	Str[256*sizeof(TCHAR)];
 TCHAR	JoystickName[128*sizeof(TCHAR)];

 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);

 PrintJoystickShortName (JoystickName,MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,-1);

 _stprintf (Str,GetResStr1(IDS_MAP_CHANGEIFMAPPING),MapWizard->IFTypeName,MapWizard->DevTypeName);
 m_TypeCaption.SetWindowText(Str);


 _stprintf (Str,GetResStr1(IDS_MAP_CHANGEJOYMAPPING),JoystickName);
 m_JoystickCaption.SetWindowText(Str);

 MapWizard->SetWizardButtons (PSWIZB_NEXT);
 return TRUE;
}

LRESULT CMapWizard_StartPage::OnWizardNext() 
{
 if (m_AddJoyMapRadio.GetCheck())
 {
  MapWizard->UnpackMapping(MAP_SCOPE_DEVICE);
  MapWizard->ChangeTypeMapping= FALSE;
  return (LRESULT) MapWizard->AxisDefPage.IDD;
 }

 if (m_AddTypeMapRadio.GetCheck())
 {
  MapWizard->UnpackMapping(MAP_SCOPE_DRIVER);
  MapWizard->ChangeTypeMapping= TRUE;
  return (LRESULT) MapWizard->AxisDefPage.IDD;
 }

 if (m_ModJoyMapRadio.GetCheck())
 {
  MapWizard->UnpackMapping(MAP_SCOPE_DEVICE);
  MapWizard->ChangeTypeMapping= FALSE;
  return (LRESULT) MapWizard->AxisDefPage.IDD;
 }

 if (m_ModTypeMapRadio.GetCheck())
 {
  MapWizard->UnpackMapping(MAP_SCOPE_DRIVER);
  MapWizard->ChangeTypeMapping= TRUE;
  return (LRESULT) MapWizard->AxisDefPage.IDD;
 }

 if (m_DelJoyMapRadio.GetCheck())
 {
  MapWizard->ChangeTypeMapping= FALSE;
  return (LRESULT) MapWizard->DelMapPage.IDD;
 }

 if (m_DelTypeMapRadio.GetCheck())
 {
  MapWizard->ChangeTypeMapping= TRUE;
  return (LRESULT) MapWizard->DelMapPage.IDD;
 }

 ASSERT(1);
 return CPropertyPage::OnWizardNext();
}

void CMapWizard_StartPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_StartPage)
	DDX_Control(pDX, IDC_TYPECAPTION, m_TypeCaption);
	DDX_Control(pDX, IDC_JOYSTICKCAPTION, m_JoystickCaption);
	DDX_Control(pDX, IDC_HELPTEXT, m_HelpText);
	DDX_Control(pDX, IDC_MODTYPEMAPRADIO, m_ModTypeMapRadio);
	DDX_Control(pDX, IDC_MODJOYMAPRADIO, m_ModJoyMapRadio);
	DDX_Control(pDX, IDC_DELTYPEMAPRADIO, m_DelTypeMapRadio);
	DDX_Control(pDX, IDC_DELJOYMAPRADIO, m_DelJoyMapRadio);
	DDX_Control(pDX, IDC_ADDTYPEMAPRADIO, m_AddTypeMapRadio);
	DDX_Control(pDX, IDC_ADDJOYMAPRADIO, m_AddJoyMapRadio);
	//}}AFX_DATA_MAP
}

BOOL CMapWizard_StartPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 m_AddTypeMapRadio.SetCheck(0);
 m_ModTypeMapRadio.SetCheck(0);
 m_DelTypeMapRadio.SetCheck(0);

 m_AddJoyMapRadio.SetCheck(0);
 m_ModJoyMapRadio.SetCheck(0);
 m_DelJoyMapRadio.SetCheck(0);

 if (MapWizard->DriverMapping)
 {
  m_AddTypeMapRadio.EnableWindow(FALSE);
  m_ModTypeMapRadio.EnableWindow(TRUE);
  m_DelTypeMapRadio.EnableWindow(TRUE);
 }
 else
 {
  m_AddTypeMapRadio.EnableWindow(TRUE);
  m_ModTypeMapRadio.EnableWindow(FALSE);
  m_DelTypeMapRadio.EnableWindow(FALSE);
 }

 if (MapWizard->DeviceMapping)
 {
  m_ModJoyMapRadio.SetCheck(1);
  m_AddJoyMapRadio.EnableWindow(FALSE);
  m_ModJoyMapRadio.EnableWindow(TRUE);
  m_DelJoyMapRadio.EnableWindow(TRUE);
 }
 else
 {
  m_AddJoyMapRadio.SetCheck(1);
  m_AddJoyMapRadio.EnableWindow(TRUE);
  m_ModJoyMapRadio.EnableWindow(FALSE);
  m_DelJoyMapRadio.EnableWindow(FALSE);
 }

 return TRUE;
}

void CMapWizard_StartPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

BEGIN_MESSAGE_MAP(CMapWizard_StartPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_StartPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_StartPage message handlers

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisMapPage property page

IMPLEMENT_DYNCREATE(CMapWizard_AxisMapPage, CPropertyPage)

CMapWizard_AxisMapPage::CMapWizard_AxisMapPage() : CPropertyPage(CMapWizard_AxisMapPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_AxisMapPage)
	//}}AFX_DATA_INIT
}

CMapWizard_AxisMapPage::~CMapWizard_AxisMapPage()
{
}

BOOL CMapWizard_AxisMapPage::OnSetActive() 
{
 int	Count;
 int	Count2;
 
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);

 m_ConfigName.SetWindowText(MapWizard->GetMapModeDescription(MapWizard->ConfigCount));

 // Set axis names
 for (Count=0;Count<8;Count++)
 {
  for (Count2=0;1;Count2++)
  {
   if (!AxisUsageList[Count2].Description)
   {
    m_AxisName[Count].SetWindowText(_T(""));
    break;
   }
   if ((AxisUsageList[Count2].Page==MapWizard->AxisDefinition[Count].Page)&&
	   (AxisUsageList[Count2].Usage==MapWizard->AxisDefinition[Count].Usage))
    break;
  }
  m_AxisName[Count].SetWindowText(AxisUsageList[Count2].Description);
 }

 UpdateControls();
 EnableControls(MapWizard->NumAxes);

 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_NEXT);
 return TRUE;
}

#if MAPWIZ_MAXAXIS>8
#error We have not defined enough axis map controls
#endif
void CMapWizard_AxisMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_AxisMapPage)
	DDX_Control(pDX, IDC_CONFIGNAME, m_ConfigName);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_A1MINCOMBO, m_AxisMinMap[0]);
	DDX_Control(pDX, IDC_A2MINCOMBO, m_AxisMinMap[1]);
	DDX_Control(pDX, IDC_A3MINCOMBO, m_AxisMinMap[2]);
	DDX_Control(pDX, IDC_A4MINCOMBO, m_AxisMinMap[3]);
	DDX_Control(pDX, IDC_A5MINCOMBO, m_AxisMinMap[4]);
	DDX_Control(pDX, IDC_A6MINCOMBO, m_AxisMinMap[5]);
	DDX_Control(pDX, IDC_A7MINCOMBO, m_AxisMinMap[6]);
	DDX_Control(pDX, IDC_A8MINCOMBO, m_AxisMinMap[7]);
	DDX_Control(pDX, IDC_A1MAXCOMBO, m_AxisMaxMap[0]);
	DDX_Control(pDX, IDC_A2MAXCOMBO, m_AxisMaxMap[1]);
	DDX_Control(pDX, IDC_A3MAXCOMBO, m_AxisMaxMap[2]);
	DDX_Control(pDX, IDC_A4MAXCOMBO, m_AxisMaxMap[3]);
	DDX_Control(pDX, IDC_A5MAXCOMBO, m_AxisMaxMap[4]);
	DDX_Control(pDX, IDC_A6MAXCOMBO, m_AxisMaxMap[5]);
	DDX_Control(pDX, IDC_A7MAXCOMBO, m_AxisMaxMap[6]);
	DDX_Control(pDX, IDC_A8MAXCOMBO, m_AxisMaxMap[7]);
	DDX_Control(pDX, IDC_A1MINSCAN, m_AxisMinScan[0]);
	DDX_Control(pDX, IDC_A2MINSCAN, m_AxisMinScan[1]);
	DDX_Control(pDX, IDC_A3MINSCAN, m_AxisMinScan[2]);
	DDX_Control(pDX, IDC_A4MINSCAN, m_AxisMinScan[3]);
	DDX_Control(pDX, IDC_A5MINSCAN, m_AxisMinScan[4]);
	DDX_Control(pDX, IDC_A6MINSCAN, m_AxisMinScan[5]);
	DDX_Control(pDX, IDC_A7MINSCAN, m_AxisMinScan[6]);
	DDX_Control(pDX, IDC_A8MINSCAN, m_AxisMinScan[7]);
	DDX_Control(pDX, IDC_A1MAXSCAN, m_AxisMaxScan[0]);
	DDX_Control(pDX, IDC_A2MAXSCAN, m_AxisMaxScan[1]);
	DDX_Control(pDX, IDC_A3MAXSCAN, m_AxisMaxScan[2]);
	DDX_Control(pDX, IDC_A4MAXSCAN, m_AxisMaxScan[3]);
	DDX_Control(pDX, IDC_A5MAXSCAN, m_AxisMaxScan[4]);
	DDX_Control(pDX, IDC_A6MAXSCAN, m_AxisMaxScan[5]);
	DDX_Control(pDX, IDC_A7MAXSCAN, m_AxisMaxScan[6]);
	DDX_Control(pDX, IDC_A8MAXSCAN, m_AxisMaxScan[7]);
	DDX_Control(pDX, IDC_AXIS1NAME, m_AxisName[0]);
	DDX_Control(pDX, IDC_AXIS2NAME, m_AxisName[1]);
	DDX_Control(pDX, IDC_AXIS3NAME, m_AxisName[2]);
	DDX_Control(pDX, IDC_AXIS4NAME, m_AxisName[3]);
	DDX_Control(pDX, IDC_AXIS5NAME, m_AxisName[4]);
	DDX_Control(pDX, IDC_AXIS6NAME, m_AxisName[5]);
	DDX_Control(pDX, IDC_AXIS7NAME, m_AxisName[6]);
	DDX_Control(pDX, IDC_AXIS8NAME, m_AxisName[7]);
}

LRESULT CMapWizard_AxisMapPage::OnWizardNext() 
{
 MapWizard->ButtonMapPage.StartOffset= 0;
 return (LRESULT) MapWizard->ButtonMapPage.IDD;
}

LRESULT CMapWizard_AxisMapPage::OnWizardBack() 
{
 int	NumButtons;

 if (MapWizard->ConfigCount>0)
 {
  MapWizard->ConfigCount--;
  if (MapWizard->NumHats>0)
   return (LRESULT) MapWizard->HatMapPage.IDD;

  NumButtons= MapWizard->NumButtons-1;
  MapWizard->ButtonMapPage.StartOffset= (NumButtons>0)?(NumButtons-(NumButtons%MAPWIZ_MAXBUTTONS)):0;

  return (LRESULT) MapWizard->ButtonMapPage.IDD;
 }
 else
  return (LRESULT) MapWizard->AxisDefPage.IDD;
}

BOOL CMapWizard_AxisMapPage::OnInitDialog()
{
 int	Count;
 int	Count2;
 TCHAR	Str[64*sizeof(TCHAR)];

 CPropertyPage::OnInitDialog();
 
 for (Count=0;Count<128;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEDIGITAL),Count);
  for (Count2=0;Count2<8;Count2++)
  {
   m_AxisMaxMap[Count2].InsertString (-1,Str);
   m_AxisMinMap[Count2].InsertString (-1,Str);
  }
 }

 for (Count=128;Count<192;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEANALOG),Count-128);
  for (Count2=0;Count2<8;Count2++)
  {
//   m_AxisMaxMap[Count2].InsertString (-1,Str);
   m_AxisMinMap[Count2].InsertString (-1,Str);
  }
 }

 for (Count=192;Count<255;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEREVANALOG),Count-192);
  for (Count2=0;Count2<8;Count2++)
  {
//   m_AxisMaxMap[Count2].InsertString (-1,Str);
   m_AxisMinMap[Count2].InsertString (-1,Str);
  }
 }

 _stprintf (Str,GetResStr1(IDS_MAP_TYPENOTHING));
 for (Count2=0;Count2<8;Count2++)
 {
  m_AxisMaxMap[Count2].InsertString (-1,Str);
  m_AxisMinMap[Count2].InsertString (-1,Str);
 }

 return TRUE;
}

void CMapWizard_AxisMapPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

#if MAPWIZ_MAXAXIS>8
#error We have not defined enough axis map controls
#endif
BEGIN_MESSAGE_MAP(CMapWizard_AxisMapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_AxisMapPage)
	ON_BN_CLICKED(IDC_A1MINSCAN, OnA1minscan)
	ON_BN_CLICKED(IDC_A2MINSCAN, OnA2minscan)
	ON_BN_CLICKED(IDC_A3MINSCAN, OnA3minscan)
	ON_BN_CLICKED(IDC_A4MINSCAN, OnA4minscan)
	ON_BN_CLICKED(IDC_A5MINSCAN, OnA5minscan)
	ON_BN_CLICKED(IDC_A6MINSCAN, OnA6minscan)
	ON_BN_CLICKED(IDC_A7MINSCAN, OnA7minscan)
	ON_BN_CLICKED(IDC_A8MINSCAN, OnA8minscan)
	ON_BN_CLICKED(IDC_A1MAXSCAN, OnA1maxscan)
	ON_BN_CLICKED(IDC_A2MAXSCAN, OnA2maxscan)
	ON_BN_CLICKED(IDC_A3MAXSCAN, OnA3maxscan)
	ON_BN_CLICKED(IDC_A4MAXSCAN, OnA4maxscan)
	ON_BN_CLICKED(IDC_A5MAXSCAN, OnA5maxscan)
	ON_BN_CLICKED(IDC_A6MAXSCAN, OnA6maxscan)
	ON_BN_CLICKED(IDC_A7MAXSCAN, OnA7maxscan)
	ON_BN_CLICKED(IDC_A8MAXSCAN, OnA8maxscan)
	ON_CBN_SELCHANGE(IDC_A1MAXCOMBO, OnSelchangeA1maxcombo)
	ON_CBN_SELCHANGE(IDC_A1MINCOMBO, OnSelchangeA1mincombo)
	ON_CBN_SELCHANGE(IDC_A2MAXCOMBO, OnSelchangeA2maxcombo)
	ON_CBN_SELCHANGE(IDC_A2MINCOMBO, OnSelchangeA2mincombo)
	ON_CBN_SELCHANGE(IDC_A3MAXCOMBO, OnSelchangeA3maxcombo)
	ON_CBN_SELCHANGE(IDC_A3MINCOMBO, OnSelchangeA3mincombo)
	ON_CBN_SELCHANGE(IDC_A4MAXCOMBO, OnSelchangeA4maxcombo)
	ON_CBN_SELCHANGE(IDC_A4MINCOMBO, OnSelchangeA4mincombo)
	ON_CBN_SELCHANGE(IDC_A5MAXCOMBO, OnSelchangeA5maxcombo)
	ON_CBN_SELCHANGE(IDC_A5MINCOMBO, OnSelchangeA5mincombo)
	ON_CBN_SELCHANGE(IDC_A6MAXCOMBO, OnSelchangeA6maxcombo)
	ON_CBN_SELCHANGE(IDC_A6MINCOMBO, OnSelchangeA6mincombo)
	ON_CBN_SELCHANGE(IDC_A7MAXCOMBO, OnSelchangeA7maxcombo)
	ON_CBN_SELCHANGE(IDC_A7MINCOMBO, OnSelchangeA7mincombo)
	ON_CBN_SELCHANGE(IDC_A8MAXCOMBO, OnSelchangeA8maxcombo)
	ON_CBN_SELCHANGE(IDC_A8MINCOMBO, OnSelchangeA8mincombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisMapPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisDefPage property page

IMPLEMENT_DYNCREATE(CMapWizard_AxisDefPage, CPropertyPage)

CMapWizard_AxisDefPage::CMapWizard_AxisDefPage() : CPropertyPage(CMapWizard_AxisDefPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_AxisDefPage)
	//}}AFX_DATA_INIT
}

CMapWizard_AxisDefPage::~CMapWizard_AxisDefPage()
{
}

BOOL CMapWizard_AxisDefPage::OnSetActive() 
{
 int	Count;
 int	Count2;
 
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);
 m_NumAxesCombo.SetCurSel(MapWizard->NumAxes);
 m_NumButtonsCombo.SetCurSel(MapWizard->NumButtons);
 m_NumHatsCombo.SetCurSel(MapWizard->NumHats);

 Count= 0;
 while (AxisUsageList[Count].Description)
 {
  for (Count2=0;Count2<8;Count2++)
   m_AxisDefine[Count2].InsertString(-1,AxisUsageList[Count].Description);
  Count++;
 }

 for (Count=0;Count<8;Count++)
 {
  for (Count2=0;1;Count2++)
  {
   if (!AxisUsageList[Count2].Description)
   {
    Count2= -1;
    break;
   }
   if ((AxisUsageList[Count2].Page==MapWizard->AxisDefinition[Count].Page)&&
	   (AxisUsageList[Count2].Usage==MapWizard->AxisDefinition[Count].Usage))
    break;
  }
  m_AxisDefine[Count].SetCurSel(Count2);
 }

 EnableControls(MapWizard->NumAxes);
 SetNextState();

 return TRUE;
}

#if MAPWIZ_MAXAXIS>8
#error We have not defined enough axis map controls
#endif
void CMapWizard_AxisDefPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_AxisDefPage)
	DDX_Control(pDX, IDC_NUMPOVHATSCOMBO, m_NumHatsCombo);
	DDX_Control(pDX, IDC_NUMBUTTONSCOMBO, m_NumButtonsCombo);
	DDX_Control(pDX, IDC_NUMAXESCOMBO, m_NumAxesCombo);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_AXIS1TYPE, m_AxisDefine[0]);
	DDX_Control(pDX, IDC_AXIS2TYPE, m_AxisDefine[1]);
	DDX_Control(pDX, IDC_AXIS3TYPE, m_AxisDefine[2]);
	DDX_Control(pDX, IDC_AXIS4TYPE, m_AxisDefine[3]);
	DDX_Control(pDX, IDC_AXIS5TYPE, m_AxisDefine[4]);
	DDX_Control(pDX, IDC_AXIS6TYPE, m_AxisDefine[5]);
	DDX_Control(pDX, IDC_AXIS7TYPE, m_AxisDefine[6]);
	DDX_Control(pDX, IDC_AXIS8TYPE, m_AxisDefine[7]);
	DDX_Control(pDX, IDC_AXIS1NAME, m_AxisName[0]);
	DDX_Control(pDX, IDC_AXIS2NAME, m_AxisName[1]);
	DDX_Control(pDX, IDC_AXIS3NAME, m_AxisName[2]);
	DDX_Control(pDX, IDC_AXIS4NAME, m_AxisName[3]);
	DDX_Control(pDX, IDC_AXIS5NAME, m_AxisName[4]);
	DDX_Control(pDX, IDC_AXIS6NAME, m_AxisName[5]);
	DDX_Control(pDX, IDC_AXIS7NAME, m_AxisName[6]);
	DDX_Control(pDX, IDC_AXIS8NAME, m_AxisName[7]);
}

LRESULT CMapWizard_AxisDefPage::OnWizardNext() 
{
 return (LRESULT) MapWizard->AxisMapPage.IDD;
}

LRESULT CMapWizard_AxisDefPage::OnWizardBack() 
{
 return (LRESULT) MapWizard->StartPage.IDD;
}

BOOL CMapWizard_AxisDefPage::OnInitDialog()
{
 int	Count;
 TCHAR	Str[16*sizeof(TCHAR)];

 CPropertyPage::OnInitDialog();

// m_NumAxesCombo.InsertString (-1,"no axes");
// m_NumAxesCombo.InsertString (-1,"1 axis");
// for (Count=2;Count<=MAPWIZ_MAXAXIS;Count++)
// {
//  sprintf (Str,"%d axes",Count);
//  m_NumAxesCombo.InsertString (-1,Str);
// }

// m_NumButtonsCombo.InsertString (-1,"no buttons");
// m_NumButtonsCombo.InsertString (-1,"1 button");
// for (Count=2;Count<=MAPWIZ_MAXBUTTONS;Count++)
// {
//  sprintf (Str,"%d buttons",Count);
//  m_NumButtonsCombo.InsertString (-1,Str);
// }

 for (Count=0;Count<=MAPWIZ_MAXAXIS;Count++)
 {
  _stprintf (Str,_T("%d"),Count);
  m_NumAxesCombo.InsertString (-1,Str);
 }

 for (Count=0;Count<=DINPUT_MAXBUTTONS;Count++)
 {
  _stprintf (Str,_T("%d"),Count);
  m_NumButtonsCombo.InsertString (-1,Str);
 }

 for (Count=0;Count<=MAPWIZ_MAXHATS;Count++)
 {
  _stprintf (Str,_T("%d"),Count);
  m_NumHatsCombo.InsertString (-1,Str);
 }

 return TRUE;
}

void CMapWizard_AxisDefPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

BEGIN_MESSAGE_MAP(CMapWizard_AxisDefPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_AxisDefPage)
	ON_CBN_SELCHANGE(IDC_AXIS8TYPE, OnSelchangeAxis8type)
	ON_CBN_SELCHANGE(IDC_AXIS7TYPE, OnSelchangeAxis7type)
	ON_CBN_SELCHANGE(IDC_AXIS6TYPE, OnSelchangeAxis6type)
	ON_CBN_SELCHANGE(IDC_AXIS5TYPE, OnSelchangeAxis5type)
	ON_CBN_SELCHANGE(IDC_AXIS4TYPE, OnSelchangeAxis4type)
	ON_CBN_SELCHANGE(IDC_AXIS3TYPE, OnSelchangeAxis3type)
	ON_CBN_SELCHANGE(IDC_AXIS2TYPE, OnSelchangeAxis2type)
	ON_CBN_SELCHANGE(IDC_AXIS1TYPE, OnSelchangeAxis1type)
	ON_CBN_SELCHANGE(IDC_NUMAXESCOMBO, OnSelchangeNumaxescombo)
	ON_CBN_SELCHANGE(IDC_NUMBUTTONSCOMBO, OnSelchangeNumbuttonscombo)
	ON_CBN_SELCHANGE(IDC_NUMPOVHATSCOMBO, OnSelchangeNumpovhatscombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_AxisDefPage message handlers
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_ButtonMapPage property page

IMPLEMENT_DYNCREATE(CMapWizard_ButtonMapPage, CPropertyPage)

CMapWizard_ButtonMapPage::CMapWizard_ButtonMapPage() : CPropertyPage(CMapWizard_ButtonMapPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_ButtonMapPage)
	//}}AFX_DATA_INIT
}

CMapWizard_ButtonMapPage::~CMapWizard_ButtonMapPage()
{
}

BOOL CMapWizard_ButtonMapPage::OnSetActive() 
{
 int	Count;

 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);

 m_ConfigName.SetWindowText(MapWizard->GetMapModeDescription(MapWizard->ConfigCount));
 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_NEXT);
 EnableControls(MapWizard->NumButtons);

 for (Count=0;Count<MAPWIZ_MAXBUTTONS;Count++)
 {
  if (Count+StartOffset<DINPUT_MAXBUTTONS)
   m_ButtonMap[Count].SetCurSel(MapWizard->ButtonMapping[MapWizard->ConfigCount][Count+StartOffset].Source);
  else
   m_ButtonMap[Count].SetCurSel(UNUSED_MAPPING);
 }
 return TRUE;
}

#if MAPWIZ_MAXBUTTON>16
#error We have not defined enough button map controls
#endif

void CMapWizard_ButtonMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_ButtonMapPage)
	DDX_Control(pDX, IDC_CONFIGNAME, m_ConfigName);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BUT1COMBO, m_ButtonMap[0]);
	DDX_Control(pDX, IDC_BUT2COMBO, m_ButtonMap[1]);
	DDX_Control(pDX, IDC_BUT3COMBO, m_ButtonMap[2]);
	DDX_Control(pDX, IDC_BUT4COMBO, m_ButtonMap[3]);
	DDX_Control(pDX, IDC_BUT5COMBO, m_ButtonMap[4]);
	DDX_Control(pDX, IDC_BUT6COMBO, m_ButtonMap[5]);
	DDX_Control(pDX, IDC_BUT7COMBO, m_ButtonMap[6]);
	DDX_Control(pDX, IDC_BUT8COMBO, m_ButtonMap[7]);
	DDX_Control(pDX, IDC_BUT9COMBO, m_ButtonMap[8]);
	DDX_Control(pDX, IDC_BUT10COMBO, m_ButtonMap[9]);
	DDX_Control(pDX, IDC_BUT11COMBO, m_ButtonMap[10]);
	DDX_Control(pDX, IDC_BUT12COMBO, m_ButtonMap[11]);
	DDX_Control(pDX, IDC_BUT13COMBO, m_ButtonMap[12]);
	DDX_Control(pDX, IDC_BUT14COMBO, m_ButtonMap[13]);
	DDX_Control(pDX, IDC_BUT15COMBO, m_ButtonMap[14]);
	DDX_Control(pDX, IDC_BUT16COMBO, m_ButtonMap[15]);
	DDX_Control(pDX, IDC_BUT1SCAN, m_ButtonScan[0]);
	DDX_Control(pDX, IDC_BUT2SCAN, m_ButtonScan[1]);
	DDX_Control(pDX, IDC_BUT3SCAN, m_ButtonScan[2]);
	DDX_Control(pDX, IDC_BUT4SCAN, m_ButtonScan[3]);
	DDX_Control(pDX, IDC_BUT5SCAN, m_ButtonScan[4]);
	DDX_Control(pDX, IDC_BUT6SCAN, m_ButtonScan[5]);
	DDX_Control(pDX, IDC_BUT7SCAN, m_ButtonScan[6]);
	DDX_Control(pDX, IDC_BUT8SCAN, m_ButtonScan[7]);
	DDX_Control(pDX, IDC_BUT9SCAN, m_ButtonScan[8]);
	DDX_Control(pDX, IDC_BUT10SCAN, m_ButtonScan[9]);
	DDX_Control(pDX, IDC_BUT11SCAN, m_ButtonScan[10]);
	DDX_Control(pDX, IDC_BUT12SCAN, m_ButtonScan[11]);
	DDX_Control(pDX, IDC_BUT13SCAN, m_ButtonScan[12]);
	DDX_Control(pDX, IDC_BUT14SCAN, m_ButtonScan[13]);
	DDX_Control(pDX, IDC_BUT15SCAN, m_ButtonScan[14]);
	DDX_Control(pDX, IDC_BUT16SCAN, m_ButtonScan[15]);
	DDX_Control(pDX, IDC_BUTTON1NAME, m_ButtonName[0]);
	DDX_Control(pDX, IDC_BUTTON2NAME, m_ButtonName[1]);
	DDX_Control(pDX, IDC_BUTTON3NAME, m_ButtonName[2]);
	DDX_Control(pDX, IDC_BUTTON4NAME, m_ButtonName[3]);
	DDX_Control(pDX, IDC_BUTTON5NAME, m_ButtonName[4]);
	DDX_Control(pDX, IDC_BUTTON6NAME, m_ButtonName[5]);
	DDX_Control(pDX, IDC_BUTTON7NAME, m_ButtonName[6]);
	DDX_Control(pDX, IDC_BUTTON8NAME, m_ButtonName[7]);
	DDX_Control(pDX, IDC_BUTTON9NAME, m_ButtonName[8]);
	DDX_Control(pDX, IDC_BUTTON10NAME, m_ButtonName[9]);
	DDX_Control(pDX, IDC_BUTTON11NAME, m_ButtonName[10]);
	DDX_Control(pDX, IDC_BUTTON12NAME, m_ButtonName[11]);
	DDX_Control(pDX, IDC_BUTTON13NAME, m_ButtonName[12]);
	DDX_Control(pDX, IDC_BUTTON14NAME, m_ButtonName[13]);
	DDX_Control(pDX, IDC_BUTTON15NAME, m_ButtonName[14]);
	DDX_Control(pDX, IDC_BUTTON16NAME, m_ButtonName[15]);
}

LRESULT CMapWizard_ButtonMapPage::OnWizardNext() 
{
 if (MapWizard->NumButtons>(StartOffset+MAPWIZ_MAXBUTTONS))
 {
  StartOffset+= MAPWIZ_MAXBUTTONS;
  return (LRESULT) MapWizard->ButtonMapPage.IDD;
 }
 
 if (MapWizard->NumHats>0)
  return (LRESULT) MapWizard->HatMapPage.IDD;

 if (MapWizard->ConfigCount<(MapWizard->NumConfigs-1))
 {
  MapWizard->ConfigCount++;
  return (LRESULT) MapWizard->AxisMapPage.IDD;
 }

 return (LRESULT) MapWizard->DonePage.IDD;
}

LRESULT CMapWizard_ButtonMapPage::OnWizardBack() 
{
 if (StartOffset>0)
 {
  StartOffset-= MAPWIZ_MAXBUTTONS;
  if (StartOffset<0)
   StartOffset= 0;

  return (LRESULT) MapWizard->ButtonMapPage.IDD;
 }

 return (LRESULT) MapWizard->AxisMapPage.IDD;
}

BOOL CMapWizard_ButtonMapPage::OnInitDialog()
{
 int	Count;
 int	Count2;
 TCHAR	Str[64*sizeof(TCHAR)];

 CPropertyPage::OnInitDialog();
 
 for (Count=0;Count<128;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEDIGITAL),Count);
  for (Count2=0;Count2<16;Count2++)
   m_ButtonMap[Count2].InsertString (-1,Str);
 }

 for (Count=128;Count<192;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEMINANALOG),Count-128);
  for (Count2=0;Count2<16;Count2++)
   m_ButtonMap[Count2].InsertString (-1,Str);
 }

 for (Count=192;Count<255;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEMAXANALOG),Count-192);
  for (Count2=0;Count2<16;Count2++)
   m_ButtonMap[Count2].InsertString (-1,Str);
 }

 _stprintf (Str,GetResStr1(IDS_MAP_TYPENOTHING));
 for (Count2=0;Count2<16;Count2++)
  m_ButtonMap[Count2].InsertString (-1,Str);

 return TRUE;
}

void CMapWizard_ButtonMapPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

#if MAPWIZ_MAXBUTTON>16
#error We have not defined enough button map controls
#endif
BEGIN_MESSAGE_MAP(CMapWizard_ButtonMapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_ButtonMapPage)
	ON_BN_CLICKED(IDC_BUT1SCAN, OnBut1scan)
	ON_BN_CLICKED(IDC_BUT2SCAN, OnBut2scan)
	ON_BN_CLICKED(IDC_BUT3SCAN, OnBut3scan)
	ON_BN_CLICKED(IDC_BUT4SCAN, OnBut4scan)
	ON_BN_CLICKED(IDC_BUT5SCAN, OnBut5scan)
	ON_BN_CLICKED(IDC_BUT6SCAN, OnBut6scan)
	ON_BN_CLICKED(IDC_BUT7SCAN, OnBut7scan)
	ON_BN_CLICKED(IDC_BUT8SCAN, OnBut8scan)
	ON_BN_CLICKED(IDC_BUT9SCAN, OnBut9scan)
	ON_BN_CLICKED(IDC_BUT10SCAN, OnBut10scan)
	ON_BN_CLICKED(IDC_BUT11SCAN, OnBut11scan)
	ON_BN_CLICKED(IDC_BUT12SCAN, OnBut12scan)
	ON_BN_CLICKED(IDC_BUT13SCAN, OnBut13scan)
	ON_BN_CLICKED(IDC_BUT14SCAN, OnBut14scan)
	ON_BN_CLICKED(IDC_BUT15SCAN, OnBut15scan)
	ON_BN_CLICKED(IDC_BUT16SCAN, OnBut16scan)
	ON_CBN_SELCHANGE(IDC_BUT10COMBO, OnSelchangeBut10combo)
	ON_CBN_SELCHANGE(IDC_BUT11COMBO, OnSelchangeBut11combo)
	ON_CBN_SELCHANGE(IDC_BUT12COMBO, OnSelchangeBut12combo)
	ON_CBN_SELCHANGE(IDC_BUT13COMBO, OnSelchangeBut13combo)
	ON_CBN_SELCHANGE(IDC_BUT14COMBO, OnSelchangeBut14combo)
	ON_CBN_SELCHANGE(IDC_BUT15COMBO, OnSelchangeBut15combo)
	ON_CBN_SELCHANGE(IDC_BUT16COMBO, OnSelchangeBut16combo)
	ON_CBN_SELCHANGE(IDC_BUT1COMBO, OnSelchangeBut1combo)
	ON_CBN_SELCHANGE(IDC_BUT2COMBO, OnSelchangeBut2combo)
	ON_CBN_SELCHANGE(IDC_BUT3COMBO, OnSelchangeBut3combo)
	ON_CBN_SELCHANGE(IDC_BUT4COMBO, OnSelchangeBut4combo)
	ON_CBN_SELCHANGE(IDC_BUT5COMBO, OnSelchangeBut5combo)
	ON_CBN_SELCHANGE(IDC_BUT6COMBO, OnSelchangeBut6combo)
	ON_CBN_SELCHANGE(IDC_BUT7COMBO, OnSelchangeBut7combo)
	ON_CBN_SELCHANGE(IDC_BUT8COMBO, OnSelchangeBut8combo)
	ON_CBN_SELCHANGE(IDC_BUT9COMBO, OnSelchangeBut9combo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_ButtonMapPage message handlers

void CMapWizard_AxisMapPage::OnA1minscan() 
{
 ScanAxis(0,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA2minscan() 
{
 ScanAxis(1,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA3minscan() 
{
 ScanAxis(2,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA4minscan() 
{
 ScanAxis(3,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA5minscan() 
{
 ScanAxis(4,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA6minscan() 
{
 ScanAxis(5,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA7minscan() 
{
 ScanAxis(6,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA8minscan() 
{
 ScanAxis(7,SCANTYPE_AXISMIN);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA1maxscan() 
{
 ScanAxis(0,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA2maxscan() 
{
 ScanAxis(1,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA3maxscan() 
{
 ScanAxis(2,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA4maxscan() 
{
 ScanAxis(3,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA5maxscan() 
{
 ScanAxis(4,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA6maxscan() 
{
 ScanAxis(5,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA7maxscan() 
{
 ScanAxis(6,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_AxisMapPage::OnA8maxscan() 
{
 ScanAxis(7,SCANTYPE_AXISMAX);		// zero-based index
}

void CMapWizard_ButtonMapPage::OnBut1scan() 
{
 ScanButton(0);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut2scan() 
{
 ScanButton(1);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut3scan() 
{
 ScanButton(2);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut4scan() 
{
 ScanButton(3);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut5scan() 
{
 ScanButton(4);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut6scan() 
{
 ScanButton(5);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut7scan() 
{
 ScanButton(6);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut8scan() 
{
 ScanButton(7);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut9scan() 
{
 ScanButton(8);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut10scan() 
{
 ScanButton(9);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut11scan() 
{
 ScanButton(10);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut12scan() 
{
 ScanButton(11);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut13scan() 
{
 ScanButton(12);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut14scan() 
{
 ScanButton(13);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut15scan() 
{
 ScanButton(14);			// Zero based index
}

void CMapWizard_ButtonMapPage::OnBut16scan() 
{
 ScanButton(15);			// Zero based index
}

/////////////////////////////////////////////////////////////////////////////
// CMapWizard

IMPLEMENT_DYNAMIC(CMapWizard, CPropertySheet)

CMapWizard::CMapWizard()
	:CPropertySheet()
{
 DefaultMapping= NULL;
 DriverMapping= NULL;
 DeviceMapping= NULL;

 StartPage.AddToWizard(this);		// Wizard starts on first page added
 AxisMapPage.AddToWizard(this);
 ButtonMapPage.AddToWizard(this);
 HatMapPage.AddToWizard(this);
 AxisDefPage.AddToWizard(this);
 DelMapPage.AddToWizard(this);
 DonePage.AddToWizard(this);

 SetWizardMode();
 SetActivePage(&StartPage);
}

//CMapWizard::CMapWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
//	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
//{
//}
//
//CMapWizard::CMapWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
//	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
//{
//}

CMapWizard::~CMapWizard()
{
 if (DefaultMapping)
  free(DefaultMapping);
 if (DriverMapping)
  free(DriverMapping);
 if (DeviceMapping)
  free(DeviceMapping);
}


BEGIN_MESSAGE_MAP(CMapWizard, CPropertySheet)
	//{{AFX_MSG_MAP(CMapWizard)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard message handlers
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DonePage property page

IMPLEMENT_DYNCREATE(CMapWizard_DonePage, CPropertyPage)

CMapWizard_DonePage::CMapWizard_DonePage() : CPropertyPage(CMapWizard_DonePage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_DonePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMapWizard_DonePage::~CMapWizard_DonePage()
{
}

void CMapWizard_DonePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_DonePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

void CMapWizard_DonePage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

BEGIN_MESSAGE_MAP(CMapWizard_DonePage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_DonePage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DonePage message handlers

BOOL CMapWizard_DonePage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);
 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
 return TRUE;
}

LRESULT CMapWizard_DonePage::OnWizardBack() 
{
 int	NumButtons; 

 if (MapWizard->NumHats>0)
  return (LRESULT) MapWizard->HatMapPage.IDD;

 NumButtons= MapWizard->NumButtons-1;
 MapWizard->ButtonMapPage.StartOffset= (NumButtons>0)?(NumButtons-(NumButtons%MAPWIZ_MAXBUTTONS)):0;

 return (LRESULT) MapWizard->ButtonMapPage.IDD;
}



void CMapWizard_AxisMapPage::EnableControls(int NumAxes)
{
 int	Count;
 int	MinMap;

 for (Count=0;Count<NumAxes;Count++)
 {
  MinMap= m_AxisMinMap[Count].GetCurSel();

  m_AxisMinMap[Count].EnableWindow(TRUE);
  m_AxisMinScan[Count].EnableWindow(TRUE);

  m_AxisMaxMap[Count].EnableWindow((MinMap<0x80)||(MinMap==UNUSED_MAPPING));
  m_AxisMaxScan[Count].EnableWindow((MinMap<0x80)||(MinMap==UNUSED_MAPPING));
  m_AxisName[Count].EnableWindow(TRUE);
 }

 for (;Count<8;Count++)
 {
  m_AxisMaxMap[Count].EnableWindow(FALSE);
  m_AxisMinMap[Count].EnableWindow(FALSE);
  m_AxisMaxScan[Count].EnableWindow(FALSE);
  m_AxisMinScan[Count].EnableWindow(FALSE);
  m_AxisName[Count].EnableWindow(FALSE);
 }
}

void CMapWizard_ButtonMapPage::EnableControls(int NumButtons)
{
 int	Count;
 TCHAR	ButtonName[64*sizeof(TCHAR)];

 for (Count=0;Count<MAPWIZ_MAXBUTTONS;Count++)
 {
  if (Count+StartOffset<NumButtons)
  {
   m_ButtonMap[Count].EnableWindow(TRUE);
   m_ButtonScan[Count].EnableWindow(TRUE);
   m_ButtonName[Count].EnableWindow(TRUE);
  }
  else
  {
   m_ButtonMap[Count].EnableWindow(FALSE);
   m_ButtonScan[Count].EnableWindow(FALSE);
   m_ButtonName[Count].EnableWindow(FALSE);
  }
  _stprintf (ButtonName,GetResStr1(IDS_MAP_TYPEBUTTON),Count+1+StartOffset);
  m_ButtonName[Count].SetWindowText(ButtonName);
 }
}

void CMapWizard_AxisDefPage::EnableControls(int NumAxes)
{
 int	Count;

//#define DONT_REDEFINE_AXIS
#ifdef DONT_REDEFINE_AXIS
 /// TEMP TEMP code until we can figure out updating joystick configs
 for (Count=0;Count<8;Count++)
 {
  m_AxisDefine[Count].EnableWindow(FALSE);
  m_AxisName[Count].EnableWindow(FALSE);
 }
 m_NumAxesCombo.EnableWindow(FALSE);
 m_NumButtonsCombo.EnableWindow(FALSE);
 return;
 /// TEMP TEMP code until we can figure out updating joystick configs
#endif

 for (Count=0;Count<NumAxes;Count++)
 {
  m_AxisDefine[Count].EnableWindow(TRUE);
  m_AxisName[Count].EnableWindow(TRUE);
 }

 for (;Count<8;Count++)
 {
  m_AxisDefine[Count].EnableWindow(FALSE);
  m_AxisName[Count].EnableWindow(FALSE);
 }
}

void CMapWizard_AxisDefPage::OnSelchangeAxis8type() 
{
 UpdateAxisMapping(7);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis7type() 
{
 UpdateAxisMapping(6);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis6type() 
{
 UpdateAxisMapping(5);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis5type() 
{
 UpdateAxisMapping(4);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis4type() 
{
 UpdateAxisMapping(3);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis3type() 
{
 UpdateAxisMapping(2);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis2type() 
{
 UpdateAxisMapping(1);	// Zero-based array number
}

void CMapWizard_AxisDefPage::OnSelchangeAxis1type() 
{
 UpdateAxisMapping(0);	// Zero-based array number
}

void CMapWizard_AxisDefPage::UpdateAxisMapping(int Axis)
{
 int	Selection;
 BYTE	Page;
 BYTE	Usage;

 Selection= m_AxisDefine[Axis].GetCurSel();
 if (Selection>=0)
 {
  Page= AxisUsageList[Selection].Page;
  Usage= AxisUsageList[Selection].Usage;
 }
 else
 {
  Page= 0;
  Usage= 0;
 }

 MapWizard->AxisDefinition[Axis].Page= Page;
 MapWizard->AxisDefinition[Axis].Usage= Usage;

 // Check (and set blank) any duplicates. Problem with keyboard select!!!
#if 0
 for (int Count=0;Count<MAPWIZ_MAXAXIS;Count++)
 {
  if (Axis==Count)
   continue;
  if (MapWizard->AxisDefinition[Count].Page!=Page)
   continue;
  if (MapWizard->AxisDefinition[Count].Usage!=Usage)
   continue;

  MapWizard->AxisDefinition[Count].Page= 0;
  MapWizard->AxisDefinition[Count].Usage= 0;
  m_AxisDefine[Count].SetCurSel(-1);
 }
#endif

 SetNextState();
}

void CMapWizard_AxisDefPage::SetNextState()
{
 int			Count;

 for (Count=0;Count<MapWizard->NumAxes;Count++)
  if (m_AxisDefine[Count].GetCurSel()==-1)
  {
   MapWizard->SetWizardButtons (PSWIZB_BACK);
   return;
  }
 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_NEXT);
}

void CMapWizard_AxisMapPage::OnSelchangeA1maxcombo() 
{
 UpdateAxisMap(0);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA1mincombo() 
{
 UpdateAxisMap(0);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA2maxcombo() 
{
 UpdateAxisMap(1);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA2mincombo() 
{
 UpdateAxisMap(1);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA3maxcombo() 
{
 UpdateAxisMap(2);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA3mincombo() 
{
 UpdateAxisMap(2);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA4maxcombo() 
{
 UpdateAxisMap(3);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA4mincombo() 
{
 UpdateAxisMap(3);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA5maxcombo() 
{
 UpdateAxisMap(4);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA5mincombo() 
{
 UpdateAxisMap(4);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA6maxcombo() 
{
 UpdateAxisMap(5);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA6mincombo() 
{
 UpdateAxisMap(5);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA7maxcombo() 
{
 UpdateAxisMap(6);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA7mincombo() 
{
 UpdateAxisMap(6);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA8maxcombo() 
{
 UpdateAxisMap(7);		// Zero based index
}

void CMapWizard_AxisMapPage::OnSelchangeA8mincombo() 
{
 UpdateAxisMap(7);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut10combo() 
{
 UpdateButtonMap(9);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut11combo() 
{
 UpdateButtonMap(10);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut12combo() 
{
 UpdateButtonMap(11);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut13combo() 
{
 UpdateButtonMap(12);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut14combo() 
{
 UpdateButtonMap(13);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut15combo() 
{
 UpdateButtonMap(14);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut16combo() 
{
 UpdateButtonMap(15);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut1combo() 
{
 UpdateButtonMap(0);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut2combo() 
{
 UpdateButtonMap(1);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut3combo() 
{
 UpdateButtonMap(2);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut4combo() 
{
 UpdateButtonMap(3);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut5combo() 
{
 UpdateButtonMap(4);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut6combo() 
{
 UpdateButtonMap(5);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut7combo() 
{
 UpdateButtonMap(6);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut8combo() 
{
 UpdateButtonMap(7);		// Zero based index
}

void CMapWizard_ButtonMapPage::OnSelchangeBut9combo() 
{
 UpdateButtonMap(8);		// Zero based index
}

void CMapWizard_ButtonMapPage::UpdateButtonMap(int Button)
{
 MapWizard->ButtonMapping[MapWizard->ConfigCount][Button+StartOffset].Source= m_ButtonMap[Button].GetCurSel();
}

void CMapWizard_AxisMapPage::UpdateAxisMap(int Axis)
{
 int	Sel;

 Sel= m_AxisMinMap[Axis].GetCurSel();
 if (Sel>=0)
  MapWizard->AxisMapping[MapWizard->ConfigCount][Axis].SourceMin= Sel;

 if ((Sel<128)||(Sel==UNUSED_MAPPING))
 {
  Sel= m_AxisMaxMap[Axis].GetCurSel();
  if (Sel==128)
   Sel= UNUSED_MAPPING;
  if (Sel>=0)
   MapWizard->AxisMapping[MapWizard->ConfigCount][Axis].SourceMax= Sel;
 }
 else
 {
  MapWizard->AxisMapping[MapWizard->ConfigCount][Axis].SourceMax= UNUSED_MAPPING;
 }

 UpdateControls();
 EnableControls (MapWizard->NumAxes);
}

void CMapWizard_AxisMapPage::UpdateControls()
{

 int	Count;
 BYTE	MinMap;
 BYTE	MaxMap;

 // Set axis mappings
 for (Count=0;Count<MAPWIZ_MAXAXIS;Count++)
 {
  MinMap= MapWizard->AxisMapping[MapWizard->ConfigCount][Count].SourceMin;
  m_AxisMinMap[Count].SetCurSel(MinMap);

  if ((MinMap<128)||(MinMap==UNUSED_MAPPING))
  {
   MaxMap= MapWizard->AxisMapping[MapWizard->ConfigCount][Count].SourceMax;
   if (MaxMap==UNUSED_MAPPING)
    m_AxisMaxMap[Count].SetCurSel(128);
   else if (MaxMap<128)
    m_AxisMaxMap[Count].SetCurSel(MaxMap);
   else
    m_AxisMaxMap[Count].SetCurSel(-1);
  }
  else
   m_AxisMaxMap[Count].SetCurSel(-1);
 }
}

int CMapWizard::DoModal() 
{
 DWORD			Count;
 CWaitDialog	WaitDialog;
 TCHAR			DeviceID[256*sizeof(TCHAR)];

 LPTNumber= pEnumData->Joysticks[JoyIndex].LPTNumber;
 JoyType= pEnumData->Joysticks[JoyIndex].JoyType;
 UnitNumber= pEnumData->Joysticks[JoyIndex].UnitNumber;
 IFTypeName= GetJoyTypeName(JoyType);
 DevTypeName= GetDevTypeName(GetDevTypeFromJoyType(JoyType));

 ReadJoystickMappings (LPTNumber,JoyType,UnitNumber,&DefaultMapping,&DriverMapping,&DeviceMapping);
 if (!DefaultMapping)
 {
  MessageBox (GetResStr1(IDS_MAP_CANTREADMAP),GetResStr2(IDS_MSG_ERROR));
  return IDCANCEL;
 }
 
 if (CPropertySheet::DoModal()==IDCANCEL)
  return IDCANCEL;

 WaitDialog.Create(WaitDialog.IDD);

 for (Count=0;Count<pEnumData->Count;Count++)
  if (((JoyType==pEnumData->Joysticks[Count].JoyType)&&(ChangeTypeMapping))||
      (((DWORD)LPTNumber==pEnumData->Joysticks[Count].LPTNumber)&&(UnitNumber==pEnumData->Joysticks[Count].UnitNumber)))
  {
   MakeDeviceID (DeviceID,pEnumData->Joysticks[Count].VendorID,pEnumData->Joysticks[Count].ProductID);
   RestartDevice (DeviceID);
  }

 WaitDialog.DestroyWindow();

 return IDOK;
}

void CMapWizard::UnpackMapping(int Scope)
{
 PJOYSTICK_MAP	JoyMap;
 int			Count1;
 int			Count2;
 BYTE			*Next;

 memset (AxisDefinition,0,sizeof(AxisDefinition));
 memset (ButtonDefinition,0,sizeof(ButtonDefinition));
 memset (POVHatDefinition,0,sizeof(POVHatDefinition));

 memset (AxisMapping,UNUSED_MAPPING,sizeof(AxisMapping));
 memset (ButtonMapping,UNUSED_MAPPING,sizeof(ButtonMapping));
 memset (POVHatMapping,UNUSED_MAPPING,sizeof(POVHatMapping));

 ConfigCount= 0;

 NumAxes= NumButtons= 0;
 NumConfigs= 1;

 JoyMap= NULL;
 switch (Scope)
 {
  case MAP_SCOPE_DEVICE:
			JoyMap= DeviceMapping;
			if (JoyMap)
			 break;

  case MAP_SCOPE_DRIVER:
			JoyMap= DriverMapping;
			if (JoyMap)
			 break;

  case MAP_SCOPE_DEFAULT:
			JoyMap= DefaultMapping;
 }
 
 if (!JoyMap)
  return;
 
 NumAxes= JoyMap->NumAxes;
 NumButtons= JoyMap->NumButtons;
 NumHats= JoyMap->NumHats;
 NumConfigs= JoyMap->NumMaps;

 Next= JoyMap->Data;
 for (Count1=0;Count1<NumAxes;Count1++)
 {
  AxisDefinition[Count1].Page=*(Next++);
  AxisDefinition[Count1].Usage=*(Next++);
 }
 for (Count1=0;Count1<NumButtons;Count1++)
 {
  ButtonDefinition[Count1].Page=*(Next++);
  ButtonDefinition[Count1].Usage=*(Next++);
 }
 for (Count1=0;Count1<JoyMap->NumHats;Count1++)
 {
  POVHatDefinition[Count1].Page=*(Next++);
  POVHatDefinition[Count1].Usage=*(Next++);
 }

 for (Count2=0;Count2<NumConfigs;Count2++)
 {
  for (Count1=0;Count1<NumAxes;Count1++)
  {
   AxisMapping[Count2][Count1].SourceMin= *(Next++);
   AxisMapping[Count2][Count1].SourceMax= *(Next++);
  }
  for (Count1=0;Count1<NumButtons;Count1++)
  {
   ButtonMapping[Count2][Count1].Source= *(Next++);
  }
  for (Count1=0;Count1<NumHats;Count1++)
  {
   POVHatMapping[Count2][Count1].SourceU= *(Next++);
   POVHatMapping[Count2][Count1].SourceD= *(Next++);
   POVHatMapping[Count2][Count1].SourceL= *(Next++);
   POVHatMapping[Count2][Count1].SourceR= *(Next++);
  }
 }
}

int CMapWizard::WriteUnpackedMapping (int Scope)
{
 BYTE	MapBuffer[4+MAPWIZ_MAXAXIS*2+MAPWIZ_MAXBUTTONS*2+MAPWIZ_MAXCONFIGS*(MAPWIZ_MAXAXIS*2+MAPWIZ_MAXBUTTONS)];
 BYTE	*Next;
 int	Count1;
 int	Count2;

 Next= MapBuffer;

 *(Next++)= NumAxes;
 *(Next++)= NumButtons;
 *(Next++)= NumHats;
 *(Next++)= NumConfigs;

 for (Count1=0;Count1<NumAxes;Count1++)
 {
  *(Next++)= AxisDefinition[Count1].Page;
  *(Next++)= AxisDefinition[Count1].Usage;
 }
 for (Count1=0;Count1<NumButtons;Count1++)
 {
  *(Next++)= ButtonDefinition[Count1].Page;
  *(Next++)= ButtonDefinition[Count1].Usage;
 }
 for (Count1=0;Count1<NumHats;Count1++)
 {
  *(Next++)= POVHatDefinition[Count1].Page;
  *(Next++)= POVHatDefinition[Count1].Usage;
 }

 for (Count2=0;Count2<NumConfigs;Count2++)
 {
  for (Count1=0;Count1<NumAxes;Count1++)
  {
   *(Next++)= AxisMapping[Count2][Count1].SourceMin;
   *(Next++)= AxisMapping[Count2][Count1].SourceMax;
  }
  for (Count1=0;Count1<NumButtons;Count1++)
  {
   *(Next++)= ButtonMapping[Count2][Count1].Source;
  }
  for (Count1=0;Count1<NumHats;Count1++)
  {
   *(Next++)= POVHatMapping[Count2][Count1].SourceU;
   *(Next++)= POVHatMapping[Count2][Count1].SourceD;
   *(Next++)= POVHatMapping[Count2][Count1].SourceL;
   *(Next++)= POVHatMapping[Count2][Count1].SourceR;
  }
 }

 return WriteJoystickMapping (LPTNumber,JoyType,UnitNumber,(JOYSTICK_MAP*)MapBuffer,Next-MapBuffer,Scope);
}

BOOL CMapWizard_DonePage::OnWizardFinish() 
{
 TCHAR	Msg[128*sizeof(TCHAR)];
 int	Scope;
 int	rc;

 if (MapWizard->ChangeTypeMapping)
  Scope= MAP_SCOPE_DRIVER;
 else
  Scope= MAP_SCOPE_DEVICE;
 
 rc= MapWizard->WriteUnpackedMapping(Scope);
 if (rc)
 {
  _stprintf (Msg,GetResStr1(IDS_MAP_CANTWRITEMAP),rc);
  MessageBox (Msg,GetResStr1(IDS_MSG_ERROR));
  return FALSE;
 }
 
 return CPropertyPage::OnWizardFinish();
}

int CMapWizard::DeleteMapping(int Scope)
{
 return DeleteJoystickMapping (LPTNumber,JoyType,UnitNumber,Scope);
}

void CMapWizard_ButtonMapPage::ScanButton(int Button)
{
 int			Scan;
 CScanDialog	ScanDialog;

 Scan= ScanDialog.ScanForInput(MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,SCANTYPE_BUTTON);
 if (Scan<0)
  return;

 MapWizard->ButtonMapping[MapWizard->ConfigCount][Button+StartOffset].Source= Scan;
 m_ButtonMap[Button].SetCurSel(Scan);
}

void CMapWizard_AxisMapPage::ScanAxis(int Axis, int ScanFlag)
{
 int			Scan;
 CScanDialog	ScanDialog;

 Scan= ScanDialog.ScanForInput(MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,ScanFlag);
 if (Scan<0)
  return;

 if (ScanFlag==SCANTYPE_AXISMIN)
  MapWizard->AxisMapping[MapWizard->ConfigCount][Axis].SourceMin= Scan;
 else if (ScanFlag==SCANTYPE_AXISMAX)
  MapWizard->AxisMapping[MapWizard->ConfigCount][Axis].SourceMax= Scan;
 else
  return;

 UpdateControls();
 EnableControls(MapWizard->NumAxes);
}
/////////////////////////////////////////////////////////////////////////////
// CWaitDialog dialog


CWaitDialog::CWaitDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CWaitDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWaitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitDialog)
	DDX_Control(pDX, IDC_PROGRESS, m_ProgressText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaitDialog, CDialog)
	//{{AFX_MSG_MAP(CWaitDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitDialog message handlers

TCHAR* CMapWizard::GetMapModeDescription(int MapMode)
{
 int	DevType;

 if (NumConfigs<2)
 {
  *MapModeDescription= 0;
  goto Exit;
 }

 DevType= GetDevTypeFromJoyType (JoyType);
 
 if (DevType==DEVTYPE_PSXPAD)
 {
  if (MapMode==0)
   _stprintf (MapModeDescription,GetResStr1(IDS_MAP_MODEPSXDIGITAL));
  else if (MapMode==1)
   _stprintf (MapModeDescription,GetResStr1(IDS_MAP_MODEPSXANALOG));
  else
   _stprintf (MapModeDescription,GetResStr1(IDS_MAP_MODEPSXUNKNOWN),MapMode+1);
  goto Exit;
 }

 _stprintf (MapModeDescription,GetResStr1(IDS_MAP_MODEGENERIC),MapMode+1,NumConfigs);

Exit:
 return MapModeDescription;
}

void CMapWizard_AxisDefPage::OnSelchangeNumaxescombo() 
{
 MapWizard->NumAxes= m_NumAxesCombo.GetCurSel();
 EnableControls(MapWizard->NumAxes);
 SetNextState();
}
/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DelMapPage property page

IMPLEMENT_DYNCREATE(CMapWizard_DelMapPage, CPropertyPage)

CMapWizard_DelMapPage::CMapWizard_DelMapPage() : CPropertyPage(CMapWizard_DelMapPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_DelMapPage)
	//}}AFX_DATA_INIT
}

CMapWizard_DelMapPage::~CMapWizard_DelMapPage()
{
}

void CMapWizard_DelMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_DelMapPage)
	DDX_Control(pDX, IDC_HELPTEXT, m_HelpText);
	DDX_Control(pDX, IDC_DELCAPTION, m_DelCaption);
	DDX_Control(pDX, IDC_DELETE_YES, m_YesRadio);
	DDX_Control(pDX, IDC_DELETE_NO, m_NoRadio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapWizard_DelMapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_DelMapPage)
	ON_BN_CLICKED(IDC_DELETE_YES, OnDeleteYes)
	ON_BN_CLICKED(IDC_DELETE_NO, OnDeleteNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_DelMapPage message handlers

void CMapWizard_AxisDefPage::OnSelchangeNumbuttonscombo() 
{
 int	Count;

 MapWizard->NumButtons= m_NumButtonsCombo.GetCurSel();

 for (Count=0;Count<MapWizard->NumButtons;Count++)
 {
  MapWizard->ButtonDefinition[Count].Page= HID_USAGE_PAGE_BUTTON;
  MapWizard->ButtonDefinition[Count].Usage= Count+1;		// Button numbers starts at 1
 }

 for (;Count<DINPUT_MAXBUTTONS;Count++)
 {
  MapWizard->ButtonDefinition[Count].Page= 0;
  MapWizard->ButtonDefinition[Count].Usage= 0;
 }
}

void CMapWizard_AxisDefPage::OnSelchangeNumpovhatscombo() 
{
 int	Count;

 MapWizard->NumHats= m_NumHatsCombo.GetCurSel();

 for (Count=0;Count<MapWizard->NumHats;Count++)
 {
  MapWizard->POVHatDefinition[Count].Page= HID_USAGE_PAGE_GENERIC;
  MapWizard->POVHatDefinition[Count].Usage= HID_USAGE_GENERIC_HATSWITCH;
 }

 for (;Count<MAPWIZ_MAXHATS;Count++)
 {
  MapWizard->POVHatDefinition[Count].Page= 0;
  MapWizard->POVHatDefinition[Count].Usage= 0;
 }
}

void CMapWizard_DelMapPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}

BOOL CMapWizard_DelMapPage::OnWizardFinish() 
{
 TCHAR	Msg[128*sizeof(TCHAR)];
 int	rc;

 if (MapWizard->ChangeTypeMapping)
  rc= MapWizard->DeleteMapping(MAP_SCOPE_DRIVER);
 else
  rc= MapWizard->DeleteMapping(MAP_SCOPE_DEVICE);

 if (rc)
 {
  _stprintf (Msg,GetResStr1(IDS_MSG_CANTDELETEMAP),rc);
  MessageBox (Msg,GetResStr1(IDS_MSG_ERROR));
  return FALSE;
 }
 
 return CPropertyPage::OnWizardFinish();
}

LRESULT CMapWizard_DelMapPage::OnWizardBack() 
{
 return (LRESULT) MapWizard->StartPage.IDD;
}

BOOL CMapWizard_DelMapPage::OnSetActive() 
{
 TCHAR	Str[256*sizeof(TCHAR)];
 TCHAR	JoystickName[128*sizeof(TCHAR)];

 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);
 MapWizard->SetWizardButtons (PSWIZB_BACK|(m_YesRadio.GetCheck()?PSWIZB_FINISH:PSWIZB_DISABLEDFINISH));

 PrintJoystickShortName (JoystickName,MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,-1);

 if (MapWizard->ChangeTypeMapping)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_DELETEDEFMAP),MapWizard->IFTypeName,MapWizard->DevTypeName);
  m_DelCaption.SetWindowText (Str);
  _stprintf (Str,GetResStr1(IDS_MAP_DELDEFMAPHELP),MapWizard->IFTypeName,MapWizard->DevTypeName);
  m_HelpText.SetWindowText (Str);
 }
 else
 {
  _stprintf (Str,GetResStr1(IDS_MAP_DELETECUSTMAP),JoystickName);
  m_DelCaption.SetWindowText (Str);
  _stprintf (Str,GetResStr1(IDS_MAP_DELCUSTMAPHELP),JoystickName,MapWizard->IFTypeName,MapWizard->DevTypeName);
  m_HelpText.SetWindowText (Str);
 }

 return TRUE;
}

void CMapWizard_DelMapPage::OnDeleteYes() 
{
 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_FINISH);
}

void CMapWizard_DelMapPage::OnDeleteNo() 
{
 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
}

BOOL CMapWizard_DelMapPage::OnInitDialog()
{
 CPropertyPage::OnInitDialog();

 m_NoRadio.SetCheck(1);
 m_YesRadio.SetCheck(0);

 return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_HatMapPage property page

IMPLEMENT_DYNCREATE(CMapWizard_HatMapPage, CPropertyPage)

CMapWizard_HatMapPage::CMapWizard_HatMapPage() : CPropertyPage(CMapWizard_HatMapPage::IDD)
{
	//{{AFX_DATA_INIT(CMapWizard_HatMapPage)
	//}}AFX_DATA_INIT
}

CMapWizard_HatMapPage::~CMapWizard_HatMapPage()
{
}

void CMapWizard_HatMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapWizard_HatMapPage)
	DDX_Control(pDX, IDC_DIGITALPOVRADIO2, m_DigitalRadio2);
	DDX_Control(pDX, IDC_DIGITALPOVRADIO1, m_DigitalRadio1);
	DDX_Control(pDX, IDC_ANALOGPOVRADIO2, m_AnalogRadio2);
	DDX_Control(pDX, IDC_ANALOGPOVRADIO1, m_AnalogRadio1);
	DDX_Control(pDX, IDC_WESTSCAN2, m_WestScan2);
	DDX_Control(pDX, IDC_WESTSCAN1, m_WestScan1);
	DDX_Control(pDX, IDC_WESTNAME2, m_WestName2);
	DDX_Control(pDX, IDC_WESTNAME1, m_WestName1);
	DDX_Control(pDX, IDC_WESTCOMBO2, m_WestCombo2);
	DDX_Control(pDX, IDC_WESTCOMBO1, m_WestCombo1);
	DDX_Control(pDX, IDC_SOUTHSCAN2, m_SouthScan2);
	DDX_Control(pDX, IDC_SOUTHSCAN1, m_SouthScan1);
	DDX_Control(pDX, IDC_SOUTHNAME2, m_SouthName2);
	DDX_Control(pDX, IDC_SOUTHNAME1, m_SouthName1);
	DDX_Control(pDX, IDC_SOUTHCOMBO2, m_SouthCombo2);
	DDX_Control(pDX, IDC_SOUTHCOMBO1, m_SouthCombo1);
	DDX_Control(pDX, IDC_NORTHSCAN2, m_NorthScan2);
	DDX_Control(pDX, IDC_NORTHSCAN1, m_NorthScan1);
	DDX_Control(pDX, IDC_NORTHNAME2, m_NorthName2);
	DDX_Control(pDX, IDC_NORTHNAME1, m_NorthName1);
	DDX_Control(pDX, IDC_NORTHCOMBO2, m_NorthCombo2);
	DDX_Control(pDX, IDC_NORTHCOMBO1, m_NorthCombo1);
	DDX_Control(pDX, IDC_EASTSCAN2, m_EastScan2);
	DDX_Control(pDX, IDC_EASTSCAN1, m_EastScan1);
	DDX_Control(pDX, IDC_EASTNAME2, m_EastName2);
	DDX_Control(pDX, IDC_EASTNAME1, m_EastName1);
	DDX_Control(pDX, IDC_EASTCOMBO2, m_EastCombo2);
	DDX_Control(pDX, IDC_EASTCOMBO1, m_EastCombo1);
	DDX_Control(pDX, IDC_DIALSCAN2, m_DialScan2);
	DDX_Control(pDX, IDC_DIALSCAN1, m_DialScan1);
	DDX_Control(pDX, IDC_DIALNAME2, m_DialName2);
	DDX_Control(pDX, IDC_DIALNAME1, m_DialName1);
	DDX_Control(pDX, IDC_DIALCOMBO2, m_DialCombo2);
	DDX_Control(pDX, IDC_DIALCOMBO1, m_DialCombo1);
	DDX_Control(pDX, IDC_CONFIGNAME, m_ConfigName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapWizard_HatMapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapWizard_HatMapPage)
	ON_CBN_SELCHANGE(IDC_DIALCOMBO1, OnSelchangeDialcombo1)
	ON_CBN_SELCHANGE(IDC_DIALCOMBO2, OnSelchangeDialcombo2)
	ON_CBN_SELCHANGE(IDC_EASTCOMBO1, OnSelchangeEastcombo1)
	ON_CBN_SELCHANGE(IDC_EASTCOMBO2, OnSelchangeEastcombo2)
	ON_CBN_SELCHANGE(IDC_NORTHCOMBO1, OnSelchangeNorthcombo1)
	ON_CBN_SELCHANGE(IDC_NORTHCOMBO2, OnSelchangeNorthcombo2)
	ON_CBN_SELCHANGE(IDC_SOUTHCOMBO1, OnSelchangeSouthcombo1)
	ON_CBN_SELCHANGE(IDC_SOUTHCOMBO2, OnSelchangeSouthcombo2)
	ON_CBN_SELCHANGE(IDC_WESTCOMBO1, OnSelchangeWestcombo1)
	ON_CBN_SELCHANGE(IDC_WESTCOMBO2, OnSelchangeWestcombo2)
	ON_BN_CLICKED(IDC_NORTHSCAN1, OnNorthscan1)
	ON_BN_CLICKED(IDC_NORTHSCAN2, OnNorthscan2)
	ON_BN_CLICKED(IDC_EASTSCAN1, OnEastscan1)
	ON_BN_CLICKED(IDC_EASTSCAN2, OnEastscan2)
	ON_BN_CLICKED(IDC_WESTSCAN1, OnWestscan1)
	ON_BN_CLICKED(IDC_WESTSCAN2, OnWestscan2)
	ON_BN_CLICKED(IDC_SOUTHSCAN1, OnSouthscan1)
	ON_BN_CLICKED(IDC_SOUTHSCAN2, OnSouthscan2)
	ON_BN_CLICKED(IDC_DIALSCAN1, OnDialscan1)
	ON_BN_CLICKED(IDC_DIALSCAN2, OnDialscan2)
	ON_BN_CLICKED(IDC_DIGITALPOVRADIO1, OnDigitalpovradio1)
	ON_BN_CLICKED(IDC_ANALOGPOVRADIO1, OnAnalogpovradio1)
	ON_BN_CLICKED(IDC_DIGITALPOVRADIO2, OnDigitalpovradio2)
	ON_BN_CLICKED(IDC_ANALOGPOVRADIO2, OnAnalogpovradio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapWizard_HatMapPage message handlers

void CMapWizard_HatMapPage::AddToWizard(CMapWizard *Wizard)
{
 MapWizard= Wizard;
 MapWizard->AddPage(this);
}


LRESULT CMapWizard_HatMapPage::OnWizardBack() 
{
 int	NumButtons;

 NumButtons= MapWizard->NumButtons-1;
 MapWizard->ButtonMapPage.StartOffset= (NumButtons>0)?(NumButtons-(NumButtons%MAPWIZ_MAXBUTTONS)):0;

 return (LRESULT) MapWizard->ButtonMapPage.IDD;
}

LRESULT CMapWizard_HatMapPage::OnWizardNext() 
{
 if (MapWizard->ConfigCount<(MapWizard->NumConfigs-1))
 {
  MapWizard->ConfigCount++;
  return (LRESULT) MapWizard->AxisMapPage.IDD;
 }

 return (LRESULT) MapWizard->DonePage.IDD;
}

BOOL CMapWizard_HatMapPage::OnSetActive() 
{
 if (!CPropertyPage::OnSetActive())
  return FALSE;

 ASSERT(MapWizard);

 m_ConfigName.SetWindowText(MapWizard->GetMapModeDescription(MapWizard->ConfigCount));

 UpdateControls();
 EnableControls(MapWizard->NumHats);

 MapWizard->SetWizardButtons (PSWIZB_BACK|PSWIZB_NEXT);
 return TRUE;
}

void CMapWizard_HatMapPage::OnSelchangeDialcombo1() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceU= DialSelToMapping(m_DialCombo1.GetCurSel());
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceD= UNUSED_MAPPING;
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceL= UNUSED_MAPPING;
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceR= UNUSED_MAPPING;
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeDialcombo2() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceU= DialSelToMapping(m_DialCombo2.GetCurSel());
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceD= UNUSED_MAPPING;
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceL= UNUSED_MAPPING;
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceR= UNUSED_MAPPING;
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeEastcombo1() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceR= DirSelToMapping(m_EastCombo1.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeEastcombo2() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceR= DirSelToMapping(m_EastCombo2.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeNorthcombo1() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceU= DirSelToMapping(m_NorthCombo1.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeNorthcombo2() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceU= DirSelToMapping(m_NorthCombo2.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeSouthcombo1() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceD= DirSelToMapping(m_SouthCombo1.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeSouthcombo2() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceD= DirSelToMapping(m_SouthCombo2.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeWestcombo1() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceL= DirSelToMapping(m_WestCombo1.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnSelchangeWestcombo2() 
{
 MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceL= DirSelToMapping(m_WestCombo2.GetCurSel());
// UpdateControls();
}

void CMapWizard_HatMapPage::OnNorthscan1() 
{
 ScanDir (&m_NorthCombo1);
 OnSelchangeNorthcombo1();
}

void CMapWizard_HatMapPage::OnNorthscan2() 
{
 ScanDir (&m_NorthCombo2);
 OnSelchangeNorthcombo2();
}

void CMapWizard_HatMapPage::OnEastscan1() 
{
 ScanDir (&m_EastCombo1);
 OnSelchangeEastcombo1();
}

void CMapWizard_HatMapPage::OnEastscan2() 
{
 ScanDir (&m_EastCombo2);
 OnSelchangeEastcombo2();
}

void CMapWizard_HatMapPage::OnWestscan1() 
{
 ScanDir (&m_WestCombo1);
 OnSelchangeWestcombo1();
}

void CMapWizard_HatMapPage::OnWestscan2() 
{
 ScanDir (&m_WestCombo2);
 OnSelchangeWestcombo2();
}

void CMapWizard_HatMapPage::OnSouthscan1() 
{
 ScanDir (&m_SouthCombo1);
 OnSelchangeSouthcombo1();
}

void CMapWizard_HatMapPage::OnSouthscan2() 
{
 ScanDir (&m_SouthCombo2);
 OnSelchangeSouthcombo2();
}

void CMapWizard_HatMapPage::OnDialscan1() 
{
 ScanDial (&m_DialCombo1);
 OnSelchangeDialcombo1();
}

void CMapWizard_HatMapPage::OnDialscan2() 
{
 ScanDial (&m_DialCombo2);
 OnSelchangeDialcombo2();
}

void CMapWizard_HatMapPage::OnDigitalpovradio1() 
{
 m_DigitalRadio1.SetCheck(1);
 m_AnalogRadio1.SetCheck(0);
 OnSelchangeNorthcombo1();
 OnSelchangeEastcombo1();
 OnSelchangeWestcombo1();
 OnSelchangeSouthcombo1();

 EnableControls(MapWizard->NumHats);
}

void CMapWizard_HatMapPage::OnAnalogpovradio1() 
{
 m_DigitalRadio1.SetCheck(0);
 m_AnalogRadio1.SetCheck(1);
 OnSelchangeDialcombo1();

 EnableControls(MapWizard->NumHats);
}

void CMapWizard_HatMapPage::OnDigitalpovradio2() 
{
 m_DigitalRadio2.SetCheck(1);
 m_AnalogRadio2.SetCheck(0);
 OnSelchangeNorthcombo2();
 OnSelchangeEastcombo2();
 OnSelchangeWestcombo2();
 OnSelchangeSouthcombo2();

 EnableControls(MapWizard->NumHats);
}

void CMapWizard_HatMapPage::OnAnalogpovradio2() 
{
 m_DigitalRadio2.SetCheck(0);
 m_AnalogRadio2.SetCheck(1);
 OnSelchangeDialcombo2();

 EnableControls(MapWizard->NumHats);
}

BOOL CMapWizard_HatMapPage::OnInitDialog() 
{
 int	Count;
 TCHAR	Str[64*sizeof(TCHAR)];

 CPropertyPage::OnInitDialog();

 /* For "Dial" Combos... */ 
 for (Count=128;Count<192;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEANALOG),Count-128);
  m_DialCombo1.InsertString (-1,Str);
  m_DialCombo2.InsertString (-1,Str);
 }

 for (Count=192;Count<255;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEREVANALOG),Count-192);
  m_DialCombo1.InsertString (-1,Str);
  m_DialCombo2.InsertString (-1,Str);
 }

 _stprintf (Str,GetResStr1(IDS_MAP_TYPENOTHING));
 m_DialCombo1.InsertString (-1,Str);
 m_DialCombo2.InsertString (-1,Str);

 /* For the direction Combos... */ 
 for (Count=0;Count<128;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEDIGITAL),Count);
  m_NorthCombo1.InsertString (-1,Str);
  m_NorthCombo2.InsertString (-1,Str);
  m_EastCombo1.InsertString (-1,Str);
  m_EastCombo2.InsertString (-1,Str);
  m_WestCombo1.InsertString (-1,Str);
  m_WestCombo2.InsertString (-1,Str);
  m_SouthCombo1.InsertString (-1,Str);
  m_SouthCombo2.InsertString (-1,Str);
 }

 for (Count=128;Count<192;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEMINANALOG),Count-128);
  m_NorthCombo1.InsertString (-1,Str);
  m_NorthCombo2.InsertString (-1,Str);
  m_EastCombo1.InsertString (-1,Str);
  m_EastCombo2.InsertString (-1,Str);
  m_WestCombo1.InsertString (-1,Str);
  m_WestCombo2.InsertString (-1,Str);
  m_SouthCombo1.InsertString (-1,Str);
  m_SouthCombo2.InsertString (-1,Str);
 }

 for (Count=192;Count<255;Count++)
 {
  _stprintf (Str,GetResStr1(IDS_MAP_TYPEMAXANALOG),Count-192);
  m_NorthCombo1.InsertString (-1,Str);
  m_NorthCombo2.InsertString (-1,Str);
  m_EastCombo1.InsertString (-1,Str);
  m_EastCombo2.InsertString (-1,Str);
  m_WestCombo1.InsertString (-1,Str);
  m_WestCombo2.InsertString (-1,Str);
  m_SouthCombo1.InsertString (-1,Str);
  m_SouthCombo2.InsertString (-1,Str);
 }

 _stprintf (Str,GetResStr1(IDS_MAP_TYPENOTHING));
 m_NorthCombo1.InsertString (-1,Str);
 m_NorthCombo2.InsertString (-1,Str);
 m_EastCombo1.InsertString (-1,Str);
 m_EastCombo2.InsertString (-1,Str);
 m_WestCombo1.InsertString (-1,Str);
 m_WestCombo2.InsertString (-1,Str);
 m_SouthCombo1.InsertString (-1,Str);
 m_SouthCombo2.InsertString (-1,Str);

 return TRUE;
}

void CMapWizard_HatMapPage::UpdateControls()
{
 BYTE	MapU;
 BYTE	MapD;
 BYTE	MapL;
 BYTE	MapR;

 /* Hat 1*/
 MapU= MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceU;
 MapD= MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceD;
 MapL= MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceL;
 MapR= MapWizard->POVHatMapping[MapWizard->ConfigCount][0].SourceR;

 if ((MapU!=UNUSED_MAPPING)&&(MapU>=128)&&(MapD==UNUSED_MAPPING))
 {
  m_DigitalRadio1.SetCheck(0);
  m_NorthCombo1.SetCurSel(-1);
  m_EastCombo1.SetCurSel(-1);
  m_WestCombo1.SetCurSel(-1);
  m_SouthCombo1.SetCurSel(-1);

  m_AnalogRadio1.SetCheck(1);
  m_DialCombo1.SetCurSel(MapU-128);
 }
 else
 {
  m_DigitalRadio1.SetCheck(1);
  m_NorthCombo1.SetCurSel(MapU);
  m_EastCombo1.SetCurSel(MapR);
  m_WestCombo1.SetCurSel(MapL);
  m_SouthCombo1.SetCurSel(MapD);

  m_AnalogRadio1.SetCheck(0);
  m_DialCombo1.SetCurSel(-1);
 }

 /* Hat 2*/
 MapU= MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceU;
 MapD= MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceD;
 MapL= MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceL;
 MapR= MapWizard->POVHatMapping[MapWizard->ConfigCount][1].SourceR;

 if ((MapU!=UNUSED_MAPPING)&&(MapU>=128)&&(MapD==UNUSED_MAPPING))
 {
  m_DigitalRadio2.SetCheck(0);
  m_NorthCombo2.SetCurSel(-1);
  m_EastCombo2.SetCurSel(-1);
  m_WestCombo2.SetCurSel(-1);
  m_SouthCombo2.SetCurSel(-1);

  m_AnalogRadio2.SetCheck(1);
  m_DialCombo2.SetCurSel(MapU-128);
 }
 else
 {
  m_DigitalRadio2.SetCheck(1);
  m_NorthCombo2.SetCurSel(MapU);
  m_EastCombo2.SetCurSel(MapR);
  m_WestCombo2.SetCurSel(MapL);
  m_SouthCombo2.SetCurSel(MapD);

  m_AnalogRadio2.SetCheck(0);
  m_DialCombo2.SetCurSel(-1);
 }
}

void CMapWizard_HatMapPage::EnableControls(int NumHats)
{
 BOOL	Hat1Analog;
 BOOL	Hat1Digital;
 BOOL	Hat1Control;
 BOOL	Hat2Analog;
 BOOL	Hat2Digital;
 BOOL	Hat2Control;

 Hat1Control= NumHats>0;
 Hat2Control= NumHats>1;

 Hat1Analog= Hat1Digital= FALSE;
 Hat2Analog= Hat2Digital= FALSE;

 if (Hat1Control)
 {
  Hat1Analog= m_AnalogRadio1.GetCheck()>0;
  Hat1Digital= m_DigitalRadio1.GetCheck()>0;
 }
 if (Hat2Control)
 {
  Hat2Analog= m_AnalogRadio2.GetCheck()>0;
  Hat2Digital= m_DigitalRadio2.GetCheck()>0;
 }

 m_NorthName1.EnableWindow(Hat1Digital);
 m_EastName1.EnableWindow(Hat1Digital);
 m_WestName1.EnableWindow(Hat1Digital);
 m_SouthName1.EnableWindow(Hat1Digital);
 m_NorthCombo1.EnableWindow(Hat1Digital);
 m_EastCombo1.EnableWindow(Hat1Digital);
 m_WestCombo1.EnableWindow(Hat1Digital);
 m_SouthCombo1.EnableWindow(Hat1Digital);
 m_NorthScan1.EnableWindow(Hat1Digital);
 m_EastScan1.EnableWindow(Hat1Digital);
 m_WestScan1.EnableWindow(Hat1Digital);
 m_SouthScan1.EnableWindow(Hat1Digital);

 m_DialName1.EnableWindow(Hat1Analog);
 m_DialCombo1.EnableWindow(Hat1Analog);
 m_DialScan1.EnableWindow(Hat1Analog);

 m_DigitalRadio1.EnableWindow(Hat1Control);
 m_AnalogRadio1.EnableWindow(Hat1Control);

 m_NorthName2.EnableWindow(Hat2Digital);
 m_EastName2.EnableWindow(Hat2Digital);
 m_WestName2.EnableWindow(Hat2Digital);
 m_SouthName2.EnableWindow(Hat2Digital);
 m_NorthCombo2.EnableWindow(Hat2Digital);
 m_EastCombo2.EnableWindow(Hat2Digital);
 m_WestCombo2.EnableWindow(Hat2Digital);
 m_SouthCombo2.EnableWindow(Hat2Digital);
 m_NorthScan2.EnableWindow(Hat2Digital);
 m_EastScan2.EnableWindow(Hat2Digital);
 m_WestScan2.EnableWindow(Hat2Digital);
 m_SouthScan2.EnableWindow(Hat2Digital);

 m_DialName2.EnableWindow(Hat2Analog);
 m_DialCombo2.EnableWindow(Hat2Analog);
 m_DialScan2.EnableWindow(Hat2Analog);

 m_DigitalRadio2.EnableWindow(Hat2Control);
 m_AnalogRadio2.EnableWindow(Hat2Control);
}

BYTE CMapWizard_HatMapPage::DirSelToMapping(int Sel)
{
 if ((Sel<0)||(Sel>255)) return UNUSED_MAPPING;
 return (BYTE) Sel;
}

BYTE CMapWizard_HatMapPage::DialSelToMapping(int Sel)
{
 if ((Sel<0)||(Sel>128)) return UNUSED_MAPPING;
 return (BYTE) Sel+128;
}

void CMapWizard_HatMapPage::ScanDir (CComboBox *combo)
{
 int			Scan;
 CScanDialog	ScanDialog;

 Scan= ScanDialog.ScanForInput(MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,SCANTYPE_BUTTON);
 if (Scan<0)
  return;

 combo->SetCurSel(Scan);
}

void CMapWizard_HatMapPage::ScanDial (CComboBox *combo)
{
 int			Scan;
 CScanDialog	ScanDialog;

 Scan= ScanDialog.ScanForInput(MapWizard->LPTNumber,MapWizard->JoyType,MapWizard->UnitNumber,SCANTYPE_HATDIAL);
 if (Scan<0)
  return;

 combo->SetCurSel(Scan-128);
}
