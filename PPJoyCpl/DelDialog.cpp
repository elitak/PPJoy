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


// DelDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyCpl.h"
#include "DelDialog.h"

#include "JoyBus.h"
#include "ConfigMan.h"
#include "Registry.h"
#include "DXUtil.h"
#include "JoyDefs.h"
#include "Debug.h"
#include "ResourceStrings.h"

#include <branding.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDelDialog dialog


CDelDialog::CDelDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDelDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDelDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDelDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDelDialog)
	DDX_Control(pDX, IDC_REMOVEDINPUTCHECK, m_RemoveDInputCheck);
	DDX_Control(pDX, IDC_REMOVETEXT, m_RemoveText);
	DDX_Control(pDX, IDC_REMOVEDRIVERCHECK, m_RemoveDriverCheck);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDelDialog, CDialog)
	//{{AFX_MSG_MAP(CDelDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDelDialog message handlers

BOOL CDelDialog::OnInitDialog()
{
 TCHAR	JoystickName[128*sizeof(TCHAR)];
 TCHAR	Buffer[256*sizeof(TCHAR)];
 DWORD	LastState;
 
 CDialog::OnInitDialog();

 PrintJoystickLongName (JoystickName,pEnumData->Joysticks[DelIndex].LPTNumber,pEnumData->Joysticks[DelIndex].JoyType,
						pEnumData->Joysticks[DelIndex].UnitNumber,pEnumData->Joysticks[DelIndex].JoySubType);
 _stprintf (Buffer,GetResStr1(IDS_MSG_REMOVEJOY_FSTR),JoystickName);
 m_RemoveText.SetWindowText (Buffer);

 RegKey.Create(HKEY_CURRENT_USER,_T(CPL_APPLET_REGISTRYKEY));

 if (RegKey.QueryDWORDValue(REMOVE_DRIVER_VALUE,LastState)!=ERROR_SUCCESS)
  LastState= 1;
 m_RemoveDriverCheck.SetCheck(LastState);

 if (RegKey.QueryDWORDValue(REMOVE_DINPUT_VALUE,LastState)!=ERROR_SUCCESS)
  LastState= 1;
 m_RemoveDInputCheck.SetCheck(LastState);

 return TRUE;
} 

void CDelDialog::OnOK() 
{
 TCHAR	Buffer[256*sizeof(TCHAR)];
 USHORT	VendorID;
 USHORT	ProductID;

 VendorID= pEnumData->Joysticks[DelIndex].VendorID;
 ProductID= pEnumData->Joysticks[DelIndex].ProductID;

 /* OK, user wants to delete this joystick. */

 /* First we will remove the driver (if the user wanted us to)   */
 /* If we first unplug the joystick we cannot get the HID        */
 /* device that hangs of the Joystick PDO - at least on XP. Bug? */
 DebugPrintf ((_T("m_RemoveDriverCheck.GetCheck() = %d"),m_RemoveDriverCheck.GetCheck()))
 if (m_RemoveDriverCheck.GetCheck()==1)
 {
  /* We have been asked to delete the joystick driver as well */
  MakeDeviceID (Buffer,VendorID,ProductID);
  DeleteDeviceID (Buffer,1);
 }

 /* OK, now we unplug the joystick. */
 DeleteJoystick (pEnumData->Joysticks[DelIndex].LPTNumber,pEnumData->Joysticks[DelIndex].JoyType,pEnumData->Joysticks[DelIndex].UnitNumber);

 /* Now that the joystick is unplugged we will remove the joystick device from */
 /* the DirectInput configuration. Hope you can do it after deleting joystick  */
 /* device nodes!!!                                                            */
 if (m_RemoveDInputCheck.GetCheck()==1)
 {
  DXDeleteJoystick (VendorID,ProductID);
 }

 DeleteJoystickOEMData (VendorID,ProductID);

 RegKey.SetDWORDValue(REMOVE_DRIVER_VALUE,m_RemoveDriverCheck.GetCheck());
 RegKey.SetDWORDValue(REMOVE_DINPUT_VALUE,m_RemoveDInputCheck.GetCheck());

 CDialog::OnOK();
}

