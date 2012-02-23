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


// MainDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyCpl.h"
#include "MainDialog.h"
#include "AddDialog.h"
#include "DelDialog.h"
#include "MapWizard.h"
#include "TimingWizard.h"
#include "OptionsDialog.h"
#include "DonateDialog.h"

#include <TCHAR.h>

#include "Debug.h"
#include "JoyBus.h"
#include "JoyDefs.h"
#include "ResourceStrings.h"

#include <branding.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainDialog dialog


CMainDialog::CMainDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDialog)
	DDX_Control(pDX, IDC_OPTIONSBUTTON, m_OptionsButton);
	DDX_Control(pDX, IDC_TIMINGBUTTON, m_TimingButton);
	DDX_Control(pDX, IDC_MAPBUTTON, m_MappingButton);
	DDX_Control(pDX, IDC_DELBUTTON, m_DelButton);
	DDX_Control(pDX, IDC_ADDBUTTON, m_AddButton);
	DDX_Control(pDX, IDC_JOYLIST, m_JoyList);
	DDX_Control(pDX, IDC_MAINWINDOWTEXT, m_MainWindowCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
	//{{AFX_MSG_MAP(CMainDialog)
	ON_BN_CLICKED(IDC_ADDBUTTON, OnAddbutton)
	ON_LBN_SELCHANGE(IDC_JOYLIST, OnSelchangeJoylist)
	ON_BN_CLICKED(IDC_DELBUTTON, OnDelbutton)
	ON_BN_CLICKED(IDC_MAPBUTTON, OnMapbutton)
	ON_BN_CLICKED(IDC_TIMINGBUTTON, OnTimingbutton)
	ON_BN_CLICKED(IDC_OPTIONSBUTTON, OnOptionsbutton)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainDialog message handlers

int CMainDialog::DoModal() 
{
 int	ReturnCode;

 // First, we open the Parallel Port Joystick Bus Enumerator; then get a
 // list of all the joysticks currently defined.
 if (!OpenPPJoyBusDriver())
 {
  TCHAR	Buffer[256*sizeof(TCHAR)];

  _stprintf (Buffer,GetResStr1(IDS_ERR_NOBUSDRIVER_FSTR),_T(PRODUCT_NAME_SHORT),BUS_DRIVER_FILENAME);
  MessageBox (Buffer,GetResStr1(IDS_MSG_FATALERROR),MB_OK);
  return 1;
 }

// if (!RefreshJoystickList())
// {
//  MessageBox ("Unable to enumerate joysticks","Fatal error!",MB_OK);
//  return 1;
// }

 ReturnCode= CDialog::DoModal();

 ClosePPJoyBusDriver();

 // Now we close the bus enumerator and free any memory that we may have
 // allocated to the joystick device list.
 return ReturnCode;
}

int  CMainDialog::RefreshJoystickList (void)
{
 unsigned int	Count;
 TCHAR			Buffer[1024*sizeof(TCHAR)];
 int			ItemIndex;

 if (!RescanJoysticks())
 {
  MessageBox (GetResStr1(IDS_ERR_CANNOT_ENUMERATE),GetResStr2(IDS_MSG_FATALERROR),MB_OK);
  m_AddButton.EnableWindow (FALSE);
  m_DelButton.EnableWindow (FALSE);
  m_MappingButton.EnableWindow (FALSE);
  m_TimingButton.EnableWindow(FALSE);
  return 0;
 }

 m_JoyList.ResetContent();
 for (Count=0;Count<pEnumData->Count;Count++)
 {
  PrintJoystickLongName (Buffer,pEnumData->Joysticks[Count].LPTNumber,pEnumData->Joysticks[Count].JoyType,
							pEnumData->Joysticks[Count].UnitNumber,pEnumData->Joysticks[Count].JoySubType);
  DebugPrintf ((_T("Buffer = %s\n"),Buffer))
  ItemIndex= m_JoyList.AddString(Buffer);
  if (ItemIndex==LB_ERRSPACE)
  {
   DebugPrintf ((_T("Error: insufficient space to add item to listbox")))
   break;
  }
  /* We don't properly check for a possible LB_ERR return code. Not today */
  /* Save index into pEnumData along with item in listbox.                */
  m_JoyList.SetItemData (ItemIndex,Count);
 }
 
 m_DelButton.EnableWindow (FALSE);
 m_MappingButton.EnableWindow (FALSE);
 m_TimingButton.EnableWindow(FALSE);
 m_AddButton.EnableWindow (pEnumData->Count<16);	/* Max joystick we can generate PIDs for! */

 return 1;
}

BOOL CMainDialog::OnInitDialog()
{
 CDialog::OnInitDialog();
 
 RefreshJoystickList();

 SetWindowText (_T(CPL_APPLET_TITLE) TVER_PRODUCTVERSION_STR);
 m_MainWindowCaption.SetWindowText(_T(CPL_APPLET_WINDOWTEXT));

 SetTimer(1,500,NULL);

 return TRUE;
}

void CMainDialog::OnAddbutton() 
{
 int	Result;

 CAddDialog	AddDialog;

 Result= AddDialog.DoModal();
 if (Result==IDOK)
  RefreshJoystickList();
}

void CMainDialog::OnSelchangeJoylist() 
{
 int			CurrentSel;
 unsigned int	JoyIndex;
 
 /* Get the index of the joystick selected in the listbox. */
 /* We do very little error checking here. Perhaps later.  */
 CurrentSel= m_JoyList.GetCurSel();
 JoyIndex= m_JoyList.GetItemData(CurrentSel);

 /* Enable remove button */
 m_DelButton.EnableWindow (TRUE);
 m_MappingButton.EnableWindow (TRUE);

 switch (pEnumData->Joysticks[JoyIndex].JoyType)
 {
  case IF_GENESISLIN:
  case IF_GENESISDPP:
  case IF_GENESISDPP6:
  case IF_GENESISNTP:
  case IF_GENESISCC:
  case IF_GENESISSNES:
  case IF_SNESPADLIN:
  case IF_SNESPADDPP:
  case IF_PSXPBLIB:
  case IF_PSXDPADPRO:
  case IF_PSXLINUX:
  case IF_PSXNTPADXP:
  case IF_PSXMEGATAP:
  case IF_LPTSWITCH:
  case IF_FMSBUDDYBOX:
  case IF_POWERPADLIN:
//  case IF_TURBOGRAFX:			// May perhaps later add timing params
//  case IF_LINUXGAMECON:		// May perhaps later add timing params
	m_TimingButton.EnableWindow(TRUE);
	break;

  default:
	m_TimingButton.EnableWindow(FALSE);
	break;
 }
}

void CMainDialog::OnDelbutton() 
{
 int			CurrentSel;
 unsigned int	JoyIndex;
 int			Result;

 /* OK, user wants to delete this joystick. */

 CDelDialog	DelDialog;
 
 /* Get the index of the joystick selected in the listbox. */
 /* We do very little error checking here. Perhaps later.  */
 CurrentSel= m_JoyList.GetCurSel();
 JoyIndex= m_JoyList.GetItemData(CurrentSel);

 DelDialog.DelIndex= JoyIndex;
 
 Result= DelDialog.DoModal();
 if (Result==IDOK)
  RefreshJoystickList();
}

void CMainDialog::OnMapbutton() 
{
 int			CurrentSel;
 unsigned int	JoyIndex;
 
 CMapWizard	MapWizard;

 /* Get the index of the joystick selected in the listbox. */
 /* We do very little error checking here. Perhaps later.  */
 CurrentSel= m_JoyList.GetCurSel();
 JoyIndex= m_JoyList.GetItemData(CurrentSel);

 MapWizard.JoyIndex= JoyIndex;
 MapWizard.DoModal();
}


void CMainDialog::OnTimingbutton() 
{
	// TODO: Add your control notification handler code here
 int			CurrentSel;
 unsigned int	JoyIndex;
 
 CTimingWizard	TimingWizard;

 /* Get the index of the joystick selected in the listbox. */
 /* We do very little error checking here. Perhaps later.  */
 CurrentSel= m_JoyList.GetCurSel();
 JoyIndex= m_JoyList.GetItemData(CurrentSel);
	
 TimingWizard.JoyIndex= JoyIndex;
 TimingWizard.DoModal();
}

void CMainDialog::OnOptionsbutton() 
{
 COptionsDialog	OptionsDialog;

 OptionsDialog.DoModal();
}

 
void CMainDialog::OnTimer(UINT nIDEvent) 
{
 KillTimer (1);

#ifdef CPL_SHOWDONATEDIALOG
 DonateDialog();
#endif
}
