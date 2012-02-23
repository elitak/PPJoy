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


// AddDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyCpl.h"
#include "AddDialog.h"
#include "JoyBus.h"

#include "JoyDefs.h"
#include "Registry.h"
#include "ConfigMan.h"

#include "Debug.h"
#include "ResourceStrings.h"

#include <branding.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddDialog dialog


CAddDialog::CAddDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAddDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddDialog)
	//}}AFX_DATA_INIT
}


void CAddDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddDialog)
	DDX_Control(pDX, IDC_SUBTYPELIST, m_JoySubTypeList);
	DDX_Control(pDX, IDC_DEVLIST, m_DevTypeList);
	DDX_Control(pDX, IDOK, m_AddButton);
	DDX_Control(pDX, IDC_UNITLIST, m_JoyUnitList);
	DDX_Control(pDX, IDC_TYPELIST, m_JoyTypeList);
	DDX_Control(pDX, IDC_PORTLIST, m_PortList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddDialog, CDialog)
	//{{AFX_MSG_MAP(CAddDialog)
	ON_CBN_SELCHANGE(IDC_PORTLIST, OnSelchangePortlist)
	ON_CBN_SELCHANGE(IDC_TYPELIST, OnSelchangeTypelist)
	ON_CBN_SELCHANGE(IDC_DEVLIST, OnSelchangeDevlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAddDialog utility methods

int CAddDialog::FindAvailablePorts (void)
{
 ULONG	PortNumber;
 int	FreeUnits;

 /* Find out what parallel ports are present on our system. */
 /* Array with port numbers found. There is no specific     */
 /* correlation implied by array position and port number.  */
 /* But as it happens LPTx is in PortArray[x-1]             */
 RescanParallelPorts (PortList,MAX_PARPORTS);

#if 0
 /* FAKE LPT2-4 for test purposes*/
 for (PortNumber=2;PortNumber<=4;PortNumber++)
  PortList[PortNumber-1]= PortNumber;
#endif

 FreeUnits= 0;
 /* Now scan for availability on each of the ports */
 for (PortNumber=0;PortNumber<MAX_PARPORTS;PortNumber++)
 {
  /* Uhm, no such parallel port, skip it */
  if (!PortList[PortNumber])
   continue;
  /* Else find out what units are still available */
  if (!FindAvailableUnits(PortList[PortNumber]))
   PortList[PortNumber]= 0;
  else
   FreeUnits= 1;
 }

 return FreeUnits;
}

/* Returns Joystick type on port or zero if no joystick defined */
UCHAR CAddDialog::FindJoytypeOnPort (ULONG Port)
{
 ULONG	Count;

 /* Find out what kind of interface (if any) is present on the port */
 for (Count=0;Count<pEnumData->Count;Count++)
  if (pEnumData->Joysticks[Count].LPTNumber==Port)
   return pEnumData->Joysticks[Count].JoyType;

 return 0;
}

int CAddDialog::FindAvailableUnits (ULONG Port)
{
 ULONG	Count;
 int	JoyTypeFound;

 JoyTypeFound= 0;
  
 /* Find out what units are currently defined on this port */
 for (Count=0;Count<pEnumData->Count;Count++)
 {
  if (pEnumData->Joysticks[Count].LPTNumber==Port)
  {
   if (!JoyTypeFound)
   {
    JoyTypeFound= 1;
	FindJoytypeUnits (pEnumData->Joysticks[Count].JoyType);
   }
   UnitList[pEnumData->Joysticks[Count].UnitNumber]= 0;
  }
 }

 if (!JoyTypeFound)
 {
  /* No joystick type on this port - so return token unit */
  memset (UnitList,0,sizeof(UnitList));
  UnitList[0]= 1;
  return 1;
 }

 for (Count=0;Count<MAX_JOYUNITS;Count++)
  if (UnitList[Count])
   return Count+1;

 return 0;
}

int CAddDialog::FindJoytypeUnits (UCHAR JoyType)
{
 UCHAR	Count;
 UCHAR	MaxUnits;

 MaxUnits= GetJoyTypeMaxUnits (JoyType);
 for (Count=0;Count<MaxUnits;Count++)
  UnitList[Count]= Count+1;
 for (;Count<MAX_JOYUNITS;Count++)
  UnitList[Count]= 0;

 return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CAddDialog message handlers

BOOL CAddDialog::OnInitDialog()
{
 ULONG	Count;
 int	Index;
 _TCHAR	Buffer[256*sizeof(_TCHAR)];

 CDialog::OnInitDialog();
 
 /* We need to scan the available parallel ports (those in the system  */
 /* minus those that are fully configured). Then we need to figure all */ 
 /* valid configs and populate the dropdowns. What fun!                */
 if ((!FindAvailablePorts())&&(!FindAvailableUnits(0)))
 {
  MessageBox (GetResStr1(IDS_ERR_NO_FREE_LPT),GetResStr2(IDS_ERR_CANT_ADD_JOYSTICK),MB_ICONWARNING|MB_OK);
  OnCancel();
  return TRUE;
 }

 VendorID= JOY_VENDOR_ID;
 for (ProductID=JOY_PRODUCT_ID_BASE;ProductID<(JOY_PRODUCT_ID_BASE+16);ProductID++)
 {
  for (Count=0;Count<pEnumData->Count;Count++)
   if ((pEnumData->Joysticks[Count].VendorID==VendorID)&&(pEnumData->Joysticks[Count].ProductID==ProductID))
    break;
  if (Count==pEnumData->Count)
   break;
 }

 if (ProductID>=(JOY_PRODUCT_ID_BASE+16))
 {
  MessageBox (GetResStr1(IDS_ERR_NO_PRODUCTID),GetResStr2(IDS_ERR_CANT_ADD_JOYSTICK),MB_ICONWARNING|MB_OK);
  OnCancel();
  return TRUE;
 }

 /* Now populate the Port listbox with available ports */
 m_PortList.ResetContent();
 for (Count=0;Count<MAX_PARPORTS;Count++)
 {
  if (PortList[Count])
  {
   _stprintf (Buffer,_T("LPT%d"),PortList[Count]);
   Index= m_PortList.AddString (Buffer);
   m_PortList.SetItemData(Index,PortList[Count]);
  }
 }

 /* If we still have units available on the virtual port.  */
 /* Test separate on the port number for the virtual port. */
 if (FindAvailableUnits(0))
 {
  Index= m_PortList.AddString (GetResStr1(IDS_VIRTUALJOY_PORT_TYPE));
  m_PortList.SetItemData(Index,0);
 }

 m_PortList.EnableWindow(m_PortList.GetCount()>1?TRUE:FALSE);

 /* Get things going... */
 m_PortList.SetCurSel(0);
 OnSelchangePortlist();

 return TRUE;
}

/* OK, a new Parallel Port has been selected. Update all dependent listboxes */
void CAddDialog::OnSelchangePortlist() 
{
 ULONG	SelectedPort;
 UCHAR	JoyType;
 UCHAR	DevType;
 int	Index;

 /* First, find out what parallel port is selected: */	
 SelectedPort= m_PortList.GetItemData(m_PortList.GetCurSel());

 /* Now, is there a Joystick type already present on the port? */
 JoyType= (UCHAR) FindJoytypeOnPort (SelectedPort);

 m_DevTypeList.ResetContent();
 if (JoyType)
 {
  DevType= GetDevTypeFromJoyType(JoyType);
  Index= m_DevTypeList.AddString(GetDevTypeName(DevType));
  m_DevTypeList.SetItemData (Index,DevType);
 }
 else
 {
  for (DevType=1;DevType<=PPJOY_MAX_DEVTYPE;DevType++)
  {
   /* Filter out non-virtual types for virtual port: */
   if ((SelectedPort==0)&&
	   (!(DevType==DEVTYPE_IOCTL)))
	continue;
   /* Filter out virtual types for physical ports: */
   if ((SelectedPort!=0)&&
	   ((DevType==DEVTYPE_IOCTL)))
	continue;
   /* Filter out retired interface types: */
   if (DevType==DEVTYPE_RETIRED)
	continue;

   Index= m_DevTypeList.AddString(GetDevTypeName(DevType));
   m_DevTypeList.SetItemData (Index,DevType);
  }
 }

 m_DevTypeList.SetCurSel(0);
 m_DevTypeList.EnableWindow (m_DevTypeList.GetCount()>1?TRUE:FALSE);

 OnSelchangeDevlist();
}

void CAddDialog::OnSelchangeDevlist() 
{
 ULONG	SelectedPort;
 UCHAR	JoyType;
 UCHAR	DevType;
 int	Index;

 /* First, find out what parallel port is selected: */	
 SelectedPort= m_PortList.GetItemData(m_PortList.GetCurSel());

 /* Find out what DevTypes to list */
 DevType= (UCHAR) m_DevTypeList.GetItemData(m_DevTypeList.GetCurSel());

 /* Now, is there a Joystick type already present on the port? */
 JoyType= (UCHAR) FindJoytypeOnPort (SelectedPort);

 m_JoyTypeList.ResetContent();
 if (JoyType)
 {
  Index= m_JoyTypeList.AddString(GetJoyTypeName(JoyType));
  m_JoyTypeList.SetItemData (Index,JoyType);
 }
 else
 {
  for (JoyType=1;JoyType<=PPJOY_MAX_IFTYPE;JoyType++)
  {
   if (DevType!=GetDevTypeFromJoyType(JoyType))
	continue;
   Index= m_JoyTypeList.AddString(GetJoyTypeName(JoyType));
   m_JoyTypeList.SetItemData (Index,JoyType);
  }
 }
 m_JoyTypeList.SetCurSel(0);
 OnSelchangeTypelist();

 m_JoyTypeList.EnableWindow (m_JoyTypeList.GetCount()>1?TRUE:FALSE);
 m_AddButton.EnableWindow (m_JoyTypeList.GetCount()>0?TRUE:FALSE);
}

/* User has selected a new joystick type. We need to update the available */
/* subunits and the sub joystick type list.                               */
void CAddDialog::OnSelchangeTypelist() 
{
 UCHAR	JoyType;
 UCHAR	DevType;
 int	Count;
 int	Index;
 TCHAR	Buffer[256*sizeof(TCHAR)];
 ULONG	SelectedPort;

 /* First, find out what parallel port is selected: */	
 SelectedPort= m_PortList.GetItemData(m_PortList.GetCurSel());

  /* Now, is there a Joystick type already present on the port? */
 JoyType= (UCHAR) FindJoytypeOnPort (SelectedPort);
 if (JoyType)
 {
  /* There are already units defined on this port so the joystick type is */
  /* fixed. We check to see which units for this type is still available. */
  FindAvailableUnits (SelectedPort);
 }
 else
 {
  /* There is no previous units defined on this port, user can choose any */
  /* joystick type on this port. Show all units for that type in dropdown */
  JoyType= (UCHAR) m_JoyTypeList.GetItemData(m_JoyTypeList.GetCurSel());

  FindJoytypeUnits (JoyType);
 }

 /* Construct the unit list */
 m_JoyUnitList.ResetContent();
 for (Count=0;Count<MAX_JOYUNITS;Count++)
 {
  if (UnitList[Count])
  {
   _stprintf (Buffer,GetResStr1(IDS_CONTROLLER_FSTR),UnitList[Count]);
   Index= m_JoyUnitList.AddString(Buffer);
   m_JoyUnitList.SetItemData(Index,UnitList[Count]);
  }
 }
 m_JoyUnitList.EnableWindow (m_JoyUnitList.GetCount()>1?TRUE:FALSE);
 m_JoyUnitList.SetCurSel(0);

 /* Now construct the joystick subtype list */
 DevType= GetDevTypeFromJoyType(JoyType);
 m_JoySubTypeList.ResetContent();
 for (Count=0;SubTypeTable[Count].DevType>0;Count++)
 {
  if (SubTypeTable[Count].DevType!=DevType)
   continue;

  Index= m_JoySubTypeList.AddString(SubTypeTable[Count].Name);
  m_JoySubTypeList.SetItemData(Index,SubTypeTable[Count].SubType);
 }
 if (!m_JoySubTypeList.GetCount())
 {
  Index= m_JoySubTypeList.AddString(_T(" "));
  m_JoySubTypeList.SetItemData(Index,SUBTYPE_DEFAULT);
 }

 m_JoySubTypeList.EnableWindow (m_JoySubTypeList.GetCount()>1?TRUE:FALSE);
 m_JoySubTypeList.SetCurSel(0);
}

void CAddDialog::OnOK() 
{
 UCHAR	LPTNumber;
 UCHAR	JoyType;
 UCHAR	JoySubType;
 UCHAR	UnitNumber;

 TCHAR	JoystickName[256*sizeof(TCHAR)];
 TCHAR	Buffer[256*sizeof(TCHAR)];

 /* OK user wants to add joystick. We do so and then pass control down to */
 /* the standard OnOK so the dialog box can clean itself up and return.   */
 
 LPTNumber= (UCHAR) m_PortList.GetItemData(m_PortList.GetCurSel());
 JoyType= (UCHAR) m_JoyTypeList.GetItemData(m_JoyTypeList.GetCurSel());
 JoySubType= (UCHAR) m_JoySubTypeList.GetItemData(m_JoySubTypeList.GetCurSel());
 UnitNumber= (UCHAR) m_JoyUnitList.GetItemData(m_JoyUnitList.GetCurSel())-1;

 if (!CheckPortPreReqs(JoyType,LPTNumber))
  return;

 PrintJoystickShortName (JoystickName,LPTNumber,JoyType,UnitNumber,JoySubType);
 SetJoystickOEMName (JoystickName,VendorID,ProductID);

 MakeDeviceID (Buffer,VendorID,ProductID);
 SetFriendlyName (Buffer,JoystickName);

 if (!AddJoystick (LPTNumber,JoyType,UnitNumber,JoySubType,VendorID,ProductID))
 {
  MessageBox (GetResStr1(IDS_ERR_DRIVER_ERRROR),GetResStr2(IDS_ERR_CANT_ADD_JOYSTICK),MB_ICONWARNING|MB_OK);
  return;
 }
 
 CDialog::OnOK();
}

int CAddDialog::CheckPortPreReqs(UCHAR JoyType, UCHAR LPTNumber)
{

 /* We need to check for interrupt setup for FMS Buddybox LPT interface */
 if (JoyType==IF_FMSBUDDYBOX)
  if (!VerifyInterruptSetup(LPTNumber))
   return 0;

 return 1;
}

int CAddDialog::VerifyInterruptSetup(UCHAR LPTNumber)
{
 TCHAR	Buffer[512*sizeof(TCHAR)];
 TCHAR	DeviceName[16*sizeof(TCHAR)];
 int	IRQ;
 int	Connect;

 _stprintf (DeviceName,_T("LPT%d"),LPTNumber);

 IRQ= GetInterruptAssignment(DeviceName);

 if (IRQ==-1)
 {
  _stprintf (Buffer,GetResStr1(IDS_ERR_NOIRQ_FSTR),DeviceName);
  MessageBox (Buffer,GetResStr1(IDS_MSG_ERROR),MB_OK|MB_ICONERROR);
  return 0;
 }

 if (IRQ<=-2)
 {
  _stprintf (Buffer,GetResStr1(IDS_ERR_VERIFY_IRQ_FSTR),DeviceName,IRQ);
  MessageBox (Buffer,GetResStr1(IDS_MSG_WARNING),MB_OK|MB_ICONEXCLAMATION);
 }

 Connect= GetConnectInterruptSetting(DeviceName);
 if (Connect<0)
 {
  _stprintf (Buffer,GetResStr1(IDS_ERR_VERIFY_INT_FSTR),DeviceName);
  MessageBox (Buffer,GetResStr1(IDS_MSG_WARNING),MB_OK|MB_ICONEXCLAMATION);
  return 0;
 }

 if (Connect!=1)
 {
  _stprintf (Buffer,GetResStr1(IDS_MSG_ENABLE_INT_FSTR),DeviceName,Connect);
  if (MessageBox (Buffer,GetResStr1(IDS_MSG_ATTENTION),MB_YESNO|MB_ICONQUESTION)==IDNO)
   return 0;

  if (!SetConnectInterruptSetting(DeviceName,1))
  {
   _stprintf (Buffer,GetResStr1(IDS_ERR_UPDATE_INT_FSTR),DeviceName);
   MessageBox (Buffer,GetResStr1(IDS_MSG_ERROR),MB_OK|MB_ICONERROR);
  }
 }

 return 1;
}
