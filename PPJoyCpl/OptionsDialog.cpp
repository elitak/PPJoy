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


// OptionsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ppjoycpl.h"
#include "OptionsDialog.h"

#include <winioctl.h>
#include <TCHAR.h>

#include "JoyBus.h"

#include "ResourceStrings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog dialog


COptionsDialog::COptionsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDialog)
	DDX_Control(pDX, IDC_PORTNOALLOC, m_PortNoAlloc);
	DDX_Control(pDX, IDC_DEBUGPSX, m_DebugPSX);
	DDX_Control(pDX, IDC_DEBUGPPM, m_DebugPPM);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDialog, CDialog)
	//{{AFX_MSG_MAP(COptionsDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDialog message handlers

int COptionsDialog::DoModal() 
{
 int			Result;

 Result= CDialog::DoModal();

 return Result;
}

BOOL COptionsDialog::OnInitDialog() 
{

 CDialog::OnInitDialog();
	
 if (GetPPJoyOption (PPJOY_OPTION_RUNTIMEDEBUG,&optRuntimeDebug))
 {
  m_DebugPPM.SetCheck((optRuntimeDebug&RTDEBUG_FMS)?1:0);
  m_DebugPSX.SetCheck((optRuntimeDebug&RTDEBUG_PSXERR)?1:0);
 }
 else
 {
  MessageBox (GetResStr1(IDS_OPT_CANTREADDEBUG));
  m_DebugPPM.EnableWindow(FALSE);
  m_DebugPSX.EnableWindow(FALSE);
 }

 if (GetPPJoyOption (PPJOY_OPTION_PORTFLAGS,&optPortFlags))
 {
  m_PortNoAlloc.SetCheck((optPortFlags&PORTFLAG_NOALLOC)?1:0);
 }
 else
 {
  MessageBox (GetResStr1(IDS_OPT_CANTREADFLAGS));
  m_PortNoAlloc.EnableWindow(FALSE);
 }

 return TRUE;  // return TRUE unless you set the focus to a control
}

void COptionsDialog::OnOK() 
{
 if (m_DebugPPM.IsWindowEnabled())
 {
  optRuntimeDebug &= ~(RTDEBUG_FMS|RTDEBUG_PSXERR);
  if (m_DebugPPM.GetCheck()) optRuntimeDebug|= RTDEBUG_FMS;
  if (m_DebugPSX.GetCheck()) optRuntimeDebug|= RTDEBUG_PSXERR;
  
  if (!SetPPJoyOption(PPJOY_OPTION_RUNTIMEDEBUG,optRuntimeDebug))
  {
   MessageBox (GetResStr1(IDS_OPT_CANTWRITEDEBUG));
  }
 }

 if (m_PortNoAlloc.IsWindowEnabled())
 {
  optPortFlags &= ~(PORTFLAG_NOALLOC);
  if (m_PortNoAlloc.GetCheck()) optPortFlags|= PORTFLAG_NOALLOC;
  
  if (!SetPPJoyOption(PPJOY_OPTION_PORTFLAGS,optPortFlags))
  {
   MessageBox (GetResStr1(IDS_OPT_CANTWRITEFLAGS));
  }
 }

 CDialog::OnOK();
}
