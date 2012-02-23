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


// ScanDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ppjoycpl.h"
#include "ScanDialog.h"

#include "Mapping.h"

#include "ResourceStrings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScanDialog dialog

CScanDialog::CScanDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CScanDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScanDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CScanDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScanDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScanDialog, CDialog)
	//{{AFX_MSG_MAP(CScanDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScanDialog message handlers

int CScanDialog::ScanForInput(int Port, int JoyType, int SubAddr, int JoyScanType)
{
 ScanType= JoyScanType;

 hJoy= OpenJoystickDevice (Port,SubAddr);
 if (!hJoy)
  return -1;

 Mapping= ScanJoystickInput (hJoy,ScanType);

 if (Mapping==-4)	/* No input */
  if (DoModal()==IDCANCEL)
   Mapping= -1;

 if (Mapping<0)
  Mapping= -1;

 CloseHandle (hJoy);

 return Mapping;
}

void CScanDialog::OnTimer(UINT nIDEvent) 
{
 Mapping= ScanJoystickInput (hJoy,ScanType);
 if (Mapping!=-4)
 {
  KillTimer (1);
  EndDialog(IDOK);
 }

 // CDialog::OnTimer(nIDEvent);
}

BOOL CScanDialog::OnInitDialog()
{
 CDialog::OnInitDialog();
 
 if (!SetTimer(1,100,NULL))
 {
  MessageBox (GetResStr1(IDS_SCAN_TIMERERROR));
  EndDialog(IDCANCEL);
 }

 return TRUE;
}
