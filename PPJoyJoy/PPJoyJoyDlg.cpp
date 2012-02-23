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


// PPJoyJoyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyJoy.h"
#include "PPJoyJoyDlg.h"

/////////////////////////////////////////////////////////////////////////////
// Include files for DirectInput and PPJoy IOCTL calls                     //
/////////////////////////////////////////////////////////////////////////////
#define	DIRECTINPUT_VERSION	0x0500
#include <dinput.h>
#include <winioctl.h>
#include <ppjioctl.h>

//#include "Debug.h"
#include "afxwin.h"

#include "PPJoyAPI.h"
#include "Branding.h"

/////////////////////////////////////////////////////////////////////////////
// Define constants used in the application                                //
/////////////////////////////////////////////////////////////////////////////
#define	NUM_ANALOG	8					// Num analog values sent to PPJoy
#define	NUM_DIGITAL	16					// Num digital values sent to PPJoy
#define	PPJOYJOY_APPNAME	"PPJoyJoy"	// Section name for INI files

#define	WM_PPJOYSTATUS		WM_USER+1
#define	WM_RESCANSTICKS		WM_USER+2

/////////////////////////////////////////////////////////////////////////////
// Structure for PPJoy IOCTL calls                                         //
/////////////////////////////////////////////////////////////////////////////
#pragma pack(push,1)					// Fields MUST be byte alligned 
struct JOYSTICK_STATE
{
 unsigned long  Signature;				// Signature to identify packet
 char           NumAnalog;				// Num of analog values we pass 
 long           Axis[NUM_ANALOG];		// Analog input values 
 char           NumDigital;				// Number of digital values we pass 
 char           Digital[NUM_DIGITAL];	// Digital input values 
};
#pragma pack(pop)

/////////////////////////////////////////////////////////////////////////////
// Global variables used through-out the application                       //
/////////////////////////////////////////////////////////////////////////////

										// DirectInput related:
LPDIRECTINPUT			di;				// Main DirectInput interface
LPDIRECTINPUTDEVICE2	did[16];		// Interface to joystick devices
GUID				JoystickGUID[16];	// GUID for joystick devices
TCHAR				JoystickName[16][MAX_PATH]; 
DIJOYSTATE			JoystickData[16];	// Receives updated joystick data
int					JoystickCount;		// Number of joysticks defined in direct input

HANDLE				hTerminateEvent;	// Signal scan thread to exit
CWinThread			*ScanThread;		// Handle to scan loop thread

										// Fields for mix information
GUID				AxisJoyGUID[8];		// Joysticks to source each axis from
GUID				ButtonJoyGUID[16];	// Index of axis to use
int					AxisJoyIndex[8]=		{-1,-1,-1,-1,-1,-1,-1,-1};
										// Joysticks to source each axis from
int					ButtonJoyIndex[16]=		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
										// Index of axis to use
int					AxisAxisIndex[8]=		{-1,-1,-1,-1,-1,-1,-1,-1};
										// Joysticks to source each button from
int					ButtonButtonIndex[16]=	{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
										// Index of button to use
int					UpdateInterval;		// Milliseconds beteen joystick updates

										// PPJoy related fields
HANDLE				hVJoy= INVALID_HANDLE_VALUE;
										// Handle for IOCTL to current joystick
char				VJoyName[64];		// Name of the current joystick device
JOYSTICK_STATE		JoyState;			// Joystick data to send to PPJoy;
char				PPJoyStatus[128];	// PPJoy status message

char				INIFilename[1024];	// Last INI file used (load or save)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////////

int	UpdateVirtualJoyStatus;

void OpenPPJoyDevice (void)
{
 UpdateVirtualJoyStatus= 1;

 hVJoy= CreateFile(VJoyName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL); 

 if (hVJoy==INVALID_HANDLE_VALUE)
 {
  sprintf (PPJoyStatus,"Error %d opening joystick",GetLastError());
  PostMessage (AfxGetMainWnd()->m_hWnd,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
  return;
 }

 sprintf (PPJoyStatus,"Successfully opened joystick");
 PostMessage (AfxGetMainWnd()->m_hWnd,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
}

LRESULT CPPJoyJoyDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
{
 m_PPJoyStatus.SetWindowText (PPJoyStatus);
 return 0;
}

void InitJoyState()
{
 int			Count;

 // Initialise Joystick state
 memset (&JoyState,0,sizeof(JoyState));
 JoyState.Signature= JOYSTICK_STATE_V1;
 JoyState.NumAnalog= NUM_ANALOG;
 JoyState.NumDigital= NUM_DIGITAL;

 // Set axes to centred
 for (Count=0;Count<NUM_ANALOG;Count++)
  JoyState.Axis[Count]= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;

 // All buttons is unpressed by default
}

void UpdatePPJoy (void)
{
 DWORD			RetSize;
 DWORD			rc;

 // Make sure that the dialog window is already displayed. Else we simply return...
 if (!AfxGetMainWnd()->m_hWnd)
  return;

 if (hVJoy==INVALID_HANDLE_VALUE)
  OpenPPJoyDevice();

 if (hVJoy==INVALID_HANDLE_VALUE)
  return;

 if (!DeviceIoControl(hVJoy,IOCTL_PPORTJOY_SET_STATE,&JoyState,sizeof(JoyState),NULL,0,&RetSize,NULL))
 {
  rc= GetLastError();
  if (rc==2)
  {
   CloseHandle (hVJoy);
   hVJoy= CreateFile(VJoyName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL); 
   return;
  }
  UpdateVirtualJoyStatus= 1;
  sprintf (PPJoyStatus,"Error %d updating joystick",GetLastError());
  PostMessage (AfxGetMainWnd()->m_hWnd,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
 else if (UpdateVirtualJoyStatus)
 {
  UpdateVirtualJoyStatus= 0;
  sprintf (PPJoyStatus,"Sending joystick updates to PPJoy");
  PostMessage (AfxGetMainWnd()->m_hWnd,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
}

//////////////////////////////////////////////////////////////////////////////
// Joystick reading and update routines
//////////////////////////////////////////////////////////////////////////////

UINT ScanLoop (LPVOID pParam)
{
 int		Count;
 HRESULT	hr;

 while (WaitForSingleObject(hTerminateEvent,UpdateInterval)==WAIT_TIMEOUT)
 {
  // Poll DirectX for joystick update
  for (Count=0;Count<JoystickCount;Count++)
   did[Count]->Poll();

  // Some delay while we wait for the update?!
  // Sleep (1);

  // Update joystick position information
  for (Count=0;Count<JoystickCount;Count++)
  {
   hr= did[Count]->GetDeviceState(sizeof(JoystickData[0]),(LPVOID)(JoystickData+Count));
   if (hr==DIERR_INPUTLOST)
   {
    PostMessage (AfxGetMainWnd()->m_hWnd,WM_RESCANSTICKS,0,0);
	return 1;
   }
  }

  for (Count=0;Count<8;Count++)
   if ((AxisJoyIndex[Count]>=0)&&(AxisAxisIndex[Count]>=0))
	JoyState.Axis[Count]= ((LONG*)(JoystickData+AxisJoyIndex[Count]))[AxisAxisIndex[Count]]; 	// Treat structure members as an array...

  for (Count=0;Count<16;Count++)
   if ((ButtonJoyIndex[Count]>=0)&&(ButtonButtonIndex[Count]>=0))
    JoyState.Digital[Count]= (JoystickData[ButtonJoyIndex[Count]].rgbButtons[ButtonButtonIndex[Count]]&0x80)?1:0;

  // Update PPJoy virtual stick
  if (1)
  {
   UpdatePPJoy();
   //Beep(5000,1);
  }
 }

 return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

BOOL __stdcall DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi,LPVOID pvRef)
{
 HRESULT				hr;
 LPDIRECTINPUTDEVICE	didTemp=	NULL;
 DIPROPRANGE			dipr;


// MessageBox (AfxGetMainWnd()->m_hWnd,lpddi->tszInstanceName,"Found device",MB_OK);
//
 if (JoystickCount>=16)
  return DIENUM_STOP;

 memcpy (JoystickGUID+JoystickCount,&(lpddi->guidInstance),sizeof(GUID));
 strcpy (JoystickName[JoystickCount],lpddi->tszInstanceName);

#if COMPILING_DX8
 hr= di->CreateDevice(JoystickGUID[JoystickCount],did+JoystickCount,NULL);
#else
 hr= di->CreateDevice(JoystickGUID[JoystickCount],&didTemp,NULL);
 if (hr!=DI_OK)
 {
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot create joystick device","Error",MB_OK);
  goto Exit;
 }

 // Get a "Device 2" object required for polling 
 hr= didTemp->QueryInterface(IID_IDirectInputDevice2,(LPVOID*)did+JoystickCount);
 if(FAILED(hr))
 {
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot create joystick device2","Error",MB_OK);
  goto Exit;
 }

 if (didTemp)
 {
  didTemp->Release();
  didTemp= NULL;
 }
#endif

 hr= did[JoystickCount]->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd,DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
 if (FAILED(hr))
 {
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot set Cooperation level...","Error",MB_OK);
  goto Exit;
 }

 hr= did[JoystickCount]->SetDataFormat(&c_dfDIJoystick); 
 if (FAILED(hr))
 { 
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot set joystick data format...","Error",MB_OK);
  goto Exit;
 }

 // Set the axis ranges for the device
 dipr.diph.dwSize        = sizeof(DIPROPRANGE);
 dipr.diph.dwHeaderSize  = sizeof(dipr.diph);
// dipr.diph.dwHow         = DIPH_BYOFFSET;
// dipr.diph.dwObj         = DIJOFS_X;
 dipr.diph.dwHow         = DIPH_DEVICE;
 dipr.diph.dwObj         = 0;
 dipr.lMin               = PPJOY_AXIS_MIN;  // negative to the left/top
 dipr.lMax               = PPJOY_AXIS_MAX;  // positive to the right/bottom

 hr= did[JoystickCount]->SetProperty(DIPROP_RANGE,&dipr.diph);
 if (FAILED(hr))
 {
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot set joystick axis ranges...","Error",MB_OK);
  goto Exit;
 }

 hr= did[JoystickCount]->Acquire(); 
 if (FAILED(hr))
 { 
  MessageBox (AfxGetMainWnd()->m_hWnd,"Cannot acquire joystick...","Error",MB_OK);
  goto Exit;
 }

 JoystickCount++;
 return DIENUM_CONTINUE;

Exit:
 if (didTemp)
  didTemp->Release();
 didTemp= NULL;

 if (did[JoystickCount])
  did[JoystickCount]->Release();
 did[JoystickCount]= NULL;

 return DIENUM_CONTINUE;
}

int CPPJoyJoyDlg::ScanJoystickDevices (void)
{
 int	JoyCount;
 int	Count;
 int	Index;

 for (JoyCount=0;JoyCount<(sizeof(did)/sizeof(*did));JoyCount++)
 {
  if (did[JoyCount])
  {
   // do we want to unacquire here too??
   did[JoyCount]->Release();
  }
  did[JoyCount]= NULL;
 }

 memset (JoystickName,0,sizeof(JoystickName));
 memset (JoystickGUID,0,sizeof(JoystickGUID));
 JoystickCount= 0;

 for (Count=0;Count<8;Count++)
 {
  m_AxisJoy[Count]->ResetContent();
  Index= m_AxisJoy[Count]->AddString("<clear>");
  m_AxisJoy[Count]->SetItemData(Index,-1);
 }

 for (Count=0;Count<16;Count++)
 {
  m_ButJoy[Count]->ResetContent();
  Index= m_ButJoy[Count]->AddString("<clear>");
  m_ButJoy[Count]->SetItemData(Index,-1);
 }

 if (di->EnumDevices(DIDEVTYPE_JOYSTICK,DIEnumDevicesProc,NULL,DIEDFL_ALLDEVICES)!=DI_OK)
  return 0;

 for (JoyCount=0;JoyCount<JoystickCount;JoyCount++)
 {
  for (Count=0;Count<8;Count++)
  {
   Index= m_AxisJoy[Count]->AddString(JoystickName[JoyCount]);
   m_AxisJoy[Count]->SetItemData(Index,JoyCount);
  }

  for (Count=0;Count<16;Count++)
  {
   Index= m_ButJoy[Count]->AddString(JoystickName[JoyCount]);
   m_ButJoy[Count]->SetItemData(Index,JoyCount);
  }
 }

 return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ComponentName;
	CStatic m_ProductName;
	CStatic m_CopyrightStr;
};

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
    m_ComponentName.SetWindowText ("PPJoyJoy Version " VER_PRODUCTVERSION_STR);
	m_ProductName.SetWindowText (PRODUCT_NAME_LONG);
	m_CopyrightStr.SetWindowText (COPYRIGHT_STRING);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COMPNAME, m_ComponentName);
	DDX_Control(pDX, IDC_PRODNAME, m_ProductName);
	DDX_Control(pDX, IDC_COPYRIGHTSTR, m_CopyrightStr);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPPJoyJoyDlg dialog

CPPJoyJoyDlg::CPPJoyJoyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPPJoyJoyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPPJoyJoyDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

 m_AxisAxis[0]= &m_Axis1Axis;
 m_AxisAxis[1]= &m_Axis2Axis;
 m_AxisAxis[2]= &m_Axis3Axis;
 m_AxisAxis[3]= &m_Axis4Axis;
 m_AxisAxis[4]= &m_Axis5Axis;
 m_AxisAxis[5]= &m_Axis6Axis;
 m_AxisAxis[6]= &m_Axis7Axis;
 m_AxisAxis[7]= &m_Axis8Axis;

 m_AxisJoy[0]= &m_Axis1Joy;
 m_AxisJoy[1]= &m_Axis2Joy;
 m_AxisJoy[2]= &m_Axis3Joy;
 m_AxisJoy[3]= &m_Axis4Joy;
 m_AxisJoy[4]= &m_Axis5Joy;
 m_AxisJoy[5]= &m_Axis6Joy;
 m_AxisJoy[6]= &m_Axis7Joy;
 m_AxisJoy[7]= &m_Axis8Joy;

 m_ButJoy[0] = &m_But1Joy;
 m_ButJoy[1] = &m_But2Joy;
 m_ButJoy[2] = &m_But3Joy;
 m_ButJoy[3] = &m_But4Joy;
 m_ButJoy[4] = &m_But5Joy;
 m_ButJoy[5] = &m_But6Joy;
 m_ButJoy[6] = &m_But7Joy;
 m_ButJoy[7] = &m_But8Joy;
 m_ButJoy[8] = &m_But9Joy;
 m_ButJoy[9] = &m_But10Joy;
 m_ButJoy[10] = &m_But11Joy;
 m_ButJoy[11] = &m_But12Joy;
 m_ButJoy[12] = &m_But13Joy;
 m_ButJoy[13] = &m_But14Joy;
 m_ButJoy[14] = &m_But15Joy;
 m_ButJoy[15] = &m_But16Joy;
 
 m_ButBut[0] = &m_But1But;
 m_ButBut[1] = &m_But2But;
 m_ButBut[2] = &m_But3But;
 m_ButBut[3] = &m_But4But;
 m_ButBut[4] = &m_But5But;
 m_ButBut[5] = &m_But6But;
 m_ButBut[6] = &m_But7But;
 m_ButBut[7] = &m_But8But;
 m_ButBut[8] = &m_But9But;
 m_ButBut[9] = &m_But10But;
 m_ButBut[10] = &m_But11But;
 m_ButBut[11] = &m_But12But;
 m_ButBut[12] = &m_But13But;
 m_ButBut[13] = &m_But14But;
 m_ButBut[14] = &m_But15But;
 m_ButBut[15] = &m_But16But;
}

void CPPJoyJoyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPPJoyJoyDlg)
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	DDX_Control(pDX, IDC_INTERVALEDIT, m_IntervalEdit);
	DDX_Control(pDX, IDC_BUT9JOY, m_But9Joy);
	DDX_Control(pDX, IDC_BUT8JOY, m_But8Joy);
	DDX_Control(pDX, IDC_BUT7JOY, m_But7Joy);
	DDX_Control(pDX, IDC_BUT6JOY, m_But6Joy);
	DDX_Control(pDX, IDC_BUT5JOY, m_But5Joy);
	DDX_Control(pDX, IDC_BUT4JOY, m_But4Joy);
	DDX_Control(pDX, IDC_BUT3JOY, m_But3Joy);
	DDX_Control(pDX, IDC_BUT2JOY, m_But2Joy);
	DDX_Control(pDX, IDC_BUT1JOY, m_But1Joy);
	DDX_Control(pDX, IDC_BUT16JOY, m_But16Joy);
	DDX_Control(pDX, IDC_BUT15JOY, m_But15Joy);
	DDX_Control(pDX, IDC_BUT14JOY, m_But14Joy);
	DDX_Control(pDX, IDC_BUT13JOY, m_But13Joy);
	DDX_Control(pDX, IDC_BUT12JOY, m_But12Joy);
	DDX_Control(pDX, IDC_BUT11JOY, m_But11Joy);
	DDX_Control(pDX, IDC_BUT10JOY, m_But10Joy);
	DDX_Control(pDX, IDC_BUT9BUT, m_But9But);
	DDX_Control(pDX, IDC_BUT8BUT, m_But8But);
	DDX_Control(pDX, IDC_BUT7BUT, m_But7But);
	DDX_Control(pDX, IDC_BUT6BUT, m_But6But);
	DDX_Control(pDX, IDC_BUT5BUT, m_But5But);
	DDX_Control(pDX, IDC_BUT4BUT, m_But4But);
	DDX_Control(pDX, IDC_BUT3BUT, m_But3But);
	DDX_Control(pDX, IDC_BUT2BUT, m_But2But);
	DDX_Control(pDX, IDC_BUT1BUT, m_But1But);
	DDX_Control(pDX, IDC_BUT10BUT, m_But10But);
	DDX_Control(pDX, IDC_BUT16BUT, m_But16But);
	DDX_Control(pDX, IDC_BUT15BUT, m_But15But);
	DDX_Control(pDX, IDC_BUT14BUT, m_But14But);
	DDX_Control(pDX, IDC_BUT13BUT, m_But13But);
	DDX_Control(pDX, IDC_BUT12BUT, m_But12But);
	DDX_Control(pDX, IDC_BUT11BUT, m_But11But);
	DDX_Control(pDX, IDC_AXIS8JOY, m_Axis8Joy);
	DDX_Control(pDX, IDC_AXIS7JOY, m_Axis7Joy);
	DDX_Control(pDX, IDC_AXIS6JOY, m_Axis6Joy);
	DDX_Control(pDX, IDC_AXIS5JOY, m_Axis5Joy);
	DDX_Control(pDX, IDC_AXIS4JOY, m_Axis4Joy);
	DDX_Control(pDX, IDC_AXIS3JOY, m_Axis3Joy);
	DDX_Control(pDX, IDC_AXIS2JOY, m_Axis2Joy);
	DDX_Control(pDX, IDC_AXIS1JOY, m_Axis1Joy);
	DDX_Control(pDX, IDC_AXIS8AXIS, m_Axis8Axis);
	DDX_Control(pDX, IDC_AXIS7AXIS, m_Axis7Axis);
	DDX_Control(pDX, IDC_AXIS6AXIS, m_Axis6Axis);
	DDX_Control(pDX, IDC_AXIS5AXIS, m_Axis5Axis);
	DDX_Control(pDX, IDC_AXIS4AXIS, m_Axis4Axis);
	DDX_Control(pDX, IDC_AXIS3AXIS, m_Axis3Axis);
	DDX_Control(pDX, IDC_AXIS2AXIS, m_Axis2Axis);
	DDX_Control(pDX, IDC_AXIS1AXIS, m_Axis1Axis);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPPJoyJoyDlg, CDialog)
	//{{AFX_MSG_MAP(CPPJoyJoyDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
    ON_MESSAGE(WM_RESCANSTICKS, OnRescanSticks)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_AXIS1JOY, OnSelchangeAxis1joy)
	ON_CBN_SELCHANGE(IDC_AXIS2JOY, OnSelchangeAxis2joy)
	ON_CBN_SELCHANGE(IDC_AXIS3JOY, OnSelchangeAxis3joy)
	ON_CBN_SELCHANGE(IDC_AXIS4JOY, OnSelchangeAxis4joy)
	ON_CBN_SELCHANGE(IDC_AXIS5JOY, OnSelchangeAxis5joy)
	ON_CBN_SELCHANGE(IDC_AXIS6JOY, OnSelchangeAxis6joy)
	ON_CBN_SELCHANGE(IDC_AXIS7JOY, OnSelchangeAxis7joy)
	ON_CBN_SELCHANGE(IDC_AXIS8JOY, OnSelchangeAxis8joy)
	ON_CBN_SELCHANGE(IDC_AXIS1AXIS, OnSelchangeAxis1axis)
	ON_CBN_SELCHANGE(IDC_AXIS2AXIS, OnSelchangeAxis2axis)
	ON_CBN_SELCHANGE(IDC_AXIS3AXIS, OnSelchangeAxis3axis)
	ON_CBN_SELCHANGE(IDC_AXIS4AXIS, OnSelchangeAxis4axis)
	ON_CBN_SELCHANGE(IDC_AXIS5AXIS, OnSelchangeAxis5axis)
	ON_CBN_SELCHANGE(IDC_AXIS6AXIS, OnSelchangeAxis6axis)
	ON_CBN_SELCHANGE(IDC_AXIS7AXIS, OnSelchangeAxis7axis)
	ON_CBN_SELCHANGE(IDC_AXIS8AXIS, OnSelchangeAxis8axis)
	ON_CBN_SELCHANGE(IDC_BUT1JOY, OnSelchangeBut1joy)
	ON_CBN_SELCHANGE(IDC_BUT2JOY, OnSelchangeBut2joy)
	ON_CBN_SELCHANGE(IDC_BUT3JOY, OnSelchangeBut3joy)
	ON_CBN_SELCHANGE(IDC_BUT4JOY, OnSelchangeBut4joy)
	ON_CBN_SELCHANGE(IDC_BUT5JOY, OnSelchangeBut5joy)
	ON_CBN_SELCHANGE(IDC_BUT6JOY, OnSelchangeBut6joy)
	ON_CBN_SELCHANGE(IDC_BUT7JOY, OnSelchangeBut7joy)
	ON_CBN_SELCHANGE(IDC_BUT8JOY, OnSelchangeBut8joy)
	ON_CBN_SELCHANGE(IDC_BUT9JOY, OnSelchangeBut9joy)
	ON_CBN_SELCHANGE(IDC_BUT10JOY, OnSelchangeBut10joy)
	ON_CBN_SELCHANGE(IDC_BUT11JOY, OnSelchangeBut11joy)
	ON_CBN_SELCHANGE(IDC_BUT12JOY, OnSelchangeBut12joy)
	ON_CBN_SELCHANGE(IDC_BUT13JOY, OnSelchangeBut13joy)
	ON_CBN_SELCHANGE(IDC_BUT14JOY, OnSelchangeBut14joy)
	ON_CBN_SELCHANGE(IDC_BUT15JOY, OnSelchangeBut15joy)
	ON_CBN_SELCHANGE(IDC_BUT16JOY, OnSelchangeBut16joy)
	ON_CBN_SELCHANGE(IDC_BUT1BUT, OnSelchangeBut1but)
	ON_CBN_SELCHANGE(IDC_BUT2BUT, OnSelchangeBut2but)
	ON_CBN_SELCHANGE(IDC_BUT3BUT, OnSelchangeBut3but)
	ON_CBN_SELCHANGE(IDC_BUT4BUT, OnSelchangeBut4but)
	ON_CBN_SELCHANGE(IDC_BUT5BUT, OnSelchangeBut5but)
	ON_CBN_SELCHANGE(IDC_BUT6BUT, OnSelchangeBut6but)
	ON_CBN_SELCHANGE(IDC_BUT7BUT, OnSelchangeBut7but)
	ON_CBN_SELCHANGE(IDC_BUT8BUT, OnSelchangeBut8but)
	ON_CBN_SELCHANGE(IDC_BUT9BUT, OnSelchangeBut9but)
	ON_CBN_SELCHANGE(IDC_BUT10BUT, OnSelchangeBut10but)
	ON_CBN_SELCHANGE(IDC_BUT11BUT, OnSelchangeBut11but)
	ON_CBN_SELCHANGE(IDC_BUT12BUT, OnSelchangeBut12but)
	ON_CBN_SELCHANGE(IDC_BUT13BUT, OnSelchangeBut13but)
	ON_CBN_SELCHANGE(IDC_BUT14BUT, OnSelchangeBut14but)
	ON_CBN_SELCHANGE(IDC_BUT15BUT, OnSelchangeBut15but)
	ON_CBN_SELCHANGE(IDC_BUT16BUT, OnSelchangeBut16but)
	ON_BN_CLICKED(IDC_LOADINI, OnLoadini)
	ON_BN_CLICKED(IDC_SAVEINI, OnSaveini)
	ON_BN_CLICKED(IDC_SETINTERVAL, OnSetinterval)
	ON_BN_CLICKED(IDC_RESCAN, OnRescan)
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPPJoyJoyDlg message handlers

BOOL CPPJoyJoyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

 if (!PopulateJoyCombo())
 {
  MessageBox ("No " PRODUCT_NAME_SHORT " virtual joysticks detected\n\nYou must configure a virtual joystick in the " PRODUCT_NAME_SHORT "\nControl Panel before this application will work","Fatal error: no virtual joysticks",MB_OK|MB_ICONERROR); 
  EndDialog(IDCANCEL);
 }

 m_IntervalEdit.SetWindowText("25");
 OnSetinterval();

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();

 OnRescan();

 if (*AfxGetApp()->m_lpCmdLine)
  LoadINI (AfxGetApp()->m_lpCmdLine);

 return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPPJoyJoyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPPJoyJoyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPPJoyJoyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

int CPPJoyJoyDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
 HRESULT		hr;

 if (CDialog::OnCreate(lpCreateStruct) == -1)
  return -1;
	

 hr= DirectInputCreate(AfxGetInstanceHandle(),DIRECTINPUT_VERSION,&di,NULL);
 if (FAILED(hr))
 {
  MessageBox ("Error creating DirectInput interface");
  goto Exit;
 }

 return 0;

Exit:

 if (di)
  di->Release();
 di= NULL;

 return -1;
}

void CPPJoyJoyDlg::OnDestroy() 
{
 int	Count;

 StopScanLoop();

 for (Count=0;Count<(sizeof(did)/sizeof(*did));Count++)
 {
  if (did[Count])
   did[Count]->Release();
  did[Count]= NULL;
 }

 if (di)
  di->Release();
 di= NULL;

 CDialog::OnDestroy();
}

void CPPJoyJoyDlg::OnSelchangeAxis1joy() 
{
 UpdateAxisJoy(1);
}

void CPPJoyJoyDlg::OnSelchangeAxis2joy() 
{
 UpdateAxisJoy(2);
}

void CPPJoyJoyDlg::OnSelchangeAxis3joy() 
{
 UpdateAxisJoy(3);
}

void CPPJoyJoyDlg::OnSelchangeAxis4joy() 
{
 UpdateAxisJoy(4);
}

void CPPJoyJoyDlg::OnSelchangeAxis5joy() 
{
 UpdateAxisJoy(5);
}

void CPPJoyJoyDlg::OnSelchangeAxis6joy() 
{
 UpdateAxisJoy(6);
}

void CPPJoyJoyDlg::OnSelchangeAxis7joy() 
{
 UpdateAxisJoy(7);
}

void CPPJoyJoyDlg::OnSelchangeAxis8joy() 
{
 UpdateAxisJoy(8);
}

void CPPJoyJoyDlg::OnSelchangeAxis1axis() 
{
 AxisAxisIndex[0]= m_Axis1Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis2axis() 
{
 AxisAxisIndex[1]= m_Axis2Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis3axis() 
{
 AxisAxisIndex[2]= m_Axis3Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis4axis() 
{
 AxisAxisIndex[3]= m_Axis4Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis5axis() 
{
 AxisAxisIndex[4]= m_Axis5Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis6axis() 
{
 AxisAxisIndex[5]= m_Axis6Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis7axis() 
{
 AxisAxisIndex[6]= m_Axis7Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeAxis8axis() 
{
 AxisAxisIndex[7]= m_Axis8Axis.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut1joy() 
{
 UpdateButtonJoy(1);
}

void CPPJoyJoyDlg::OnSelchangeBut2joy() 
{
 UpdateButtonJoy(2);
}

void CPPJoyJoyDlg::OnSelchangeBut3joy() 
{
 UpdateButtonJoy(3);
}

void CPPJoyJoyDlg::OnSelchangeBut4joy() 
{
 UpdateButtonJoy(4);
}

void CPPJoyJoyDlg::OnSelchangeBut5joy() 
{
 UpdateButtonJoy(5);
}

void CPPJoyJoyDlg::OnSelchangeBut6joy() 
{
 UpdateButtonJoy(6);
}

void CPPJoyJoyDlg::OnSelchangeBut7joy() 
{
 UpdateButtonJoy(7);
}

void CPPJoyJoyDlg::OnSelchangeBut8joy() 
{
 UpdateButtonJoy(8);
}

void CPPJoyJoyDlg::OnSelchangeBut9joy() 
{
 UpdateButtonJoy(9);
}

void CPPJoyJoyDlg::OnSelchangeBut10joy() 
{
 UpdateButtonJoy(10);
}

void CPPJoyJoyDlg::OnSelchangeBut11joy() 
{
 UpdateButtonJoy(11);
}

void CPPJoyJoyDlg::OnSelchangeBut12joy() 
{
 UpdateButtonJoy(12);
}

void CPPJoyJoyDlg::OnSelchangeBut13joy() 
{
 UpdateButtonJoy(13);
}

void CPPJoyJoyDlg::OnSelchangeBut14joy() 
{
 UpdateButtonJoy(14);
}

void CPPJoyJoyDlg::OnSelchangeBut15joy() 
{
 UpdateButtonJoy(15);
}

void CPPJoyJoyDlg::OnSelchangeBut16joy() 
{
 UpdateButtonJoy(16);
}

void CPPJoyJoyDlg::OnSelchangeBut1but() 
{
 ButtonButtonIndex[0]= m_But1But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut2but() 
{
 ButtonButtonIndex[1]= m_But2But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut3but() 
{
 ButtonButtonIndex[2]= m_But3But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut4but() 
{
 ButtonButtonIndex[3]= m_But4But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut5but() 
{
 ButtonButtonIndex[4]= m_But5But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut6but() 
{
 ButtonButtonIndex[5]= m_But6But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut7but() 
{
 ButtonButtonIndex[6]= m_But7But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut8but() 
{
 ButtonButtonIndex[7]= m_But8But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut9but() 
{
 ButtonButtonIndex[8]= m_But9But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut10but() 
{
 ButtonButtonIndex[9]= m_But10But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut11but() 
{
 ButtonButtonIndex[10]= m_But11But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut12but() 
{
 ButtonButtonIndex[11]= m_But12But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut13but() 
{
 ButtonButtonIndex[12]= m_But13But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut14but() 
{
 ButtonButtonIndex[13]= m_But14But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut15but() 
{
 ButtonButtonIndex[14]= m_But15But.GetCurSel();
}

void CPPJoyJoyDlg::OnSelchangeBut16but() 
{
 ButtonButtonIndex[15]= m_But16But.GetCurSel();
}

void CPPJoyJoyDlg::OnLoadini() 
{
 CFileDialog	fd(TRUE);

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 fd.m_ofn.Flags|= OFN_FILEMUSTEXIST;

 if (fd.DoModal()==IDOK)
  LoadINI (fd.m_ofn.lpstrFile);
}

void CPPJoyJoyDlg::OnSaveini() 
{
 CFileDialog	fd(FALSE,".ini");

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 if (fd.DoModal()==IDOK)
  SaveINI (fd.m_ofn.lpstrFile);
}

void CPPJoyJoyDlg::OnSetinterval() 
{
 char	Buffer[64];
 DWORD	Interval;

 m_IntervalEdit.GetWindowText (Buffer,sizeof(Buffer));

 Interval= atoi (Buffer);
 if (Interval<5)
 {
  Interval=5;
  sprintf (Buffer,"%d",Interval);
  m_IntervalEdit.SetWindowText (Buffer);
 }

 UpdateInterval= Interval;
}

void CPPJoyJoyDlg::OnRescan() 
{
 StopScanLoop();

 ScanJoystickDevices();
 UpdateControls();

 StartScanLoop();
}

LRESULT CPPJoyJoyDlg::OnRescanSticks(WPARAM wParam, LPARAM lParam)
{
 OnRescan();
 return 0;
}

void CPPJoyJoyDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\" JOY_IOCTL_DEV_NAME,m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"PPJoyJoy joystick mixer for Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);
}

void SetComboSel (CComboBox &ComboBox, int DesiredItemData)
{
 int	Count;

 // Specifically for PPJoyJoy:
 if (DesiredItemData==-1)
 {
  ComboBox.SetCurSel(-1);
  return;
 }

 Count= ComboBox.GetCount();
 while (Count--)
  if (ComboBox.GetItemData(Count)==(DWORD)DesiredItemData)
   break;
 ComboBox.SetCurSel(Count);
}

void CPPJoyJoyDlg::UpdateControls()
{
 int	JoyCount;
 int	Count;

 for (Count=0;Count<8;Count++)
 {
  AxisJoyIndex[Count]= -1;

  for (JoyCount=0;JoyCount<JoystickCount;JoyCount++)
   if (!memcmp(JoystickGUID+JoyCount,AxisJoyGUID+Count,sizeof(GUID)))
   {
    AxisJoyIndex[Count]= JoyCount;
    break;
   }

  if (AxisJoyIndex[Count]==-1)
  {
   //AxisAxisIndex[Count]= -1;
   m_AxisAxis[Count]->SetCurSel(-1);
  }
  else
   m_AxisAxis[Count]->SetCurSel(AxisAxisIndex[Count]);

  SetComboSel(*(m_AxisJoy[Count]),AxisJoyIndex[Count]);
 }

 for (Count=0;Count<16;Count++)
 {
  ButtonJoyIndex[Count]= -1;

  for (JoyCount=0;JoyCount<JoystickCount;JoyCount++)
   if (!memcmp(JoystickGUID+JoyCount,ButtonJoyGUID+Count,sizeof(GUID)))
   {
    ButtonJoyIndex[Count]= JoyCount;
    break;
   }

  if (ButtonJoyIndex[Count]==-1)
  {
   // ButtonButtonIndex[Count]= -1;
   m_ButBut[Count]->SetCurSel(-1);
  }
  else
   m_ButBut[Count]->SetCurSel(ButtonButtonIndex[Count]);

  SetComboSel(*(m_ButJoy[Count]),ButtonJoyIndex[Count]);
 }

}

void CPPJoyJoyDlg::UpdateAxisJoy(int Axis)
{
 Axis--;
 AxisJoyIndex[Axis]= m_AxisJoy[Axis]->GetItemData(m_AxisJoy[Axis]->GetCurSel());
 if (AxisJoyIndex[Axis]<0)
 {
  memset (AxisJoyGUID+Axis,0,sizeof(GUID));
  m_AxisJoy[Axis]->SetCurSel(-1);
 }
 else
  memcpy (AxisJoyGUID+Axis,JoystickGUID+AxisJoyIndex[Axis],sizeof(GUID));
}

void CPPJoyJoyDlg::UpdateButtonJoy(int Button)
{
 Button--;
 ButtonJoyIndex[Button]= m_ButJoy[Button]->GetItemData(m_ButJoy[Button]->GetCurSel());
 if (ButtonJoyIndex[Button]<0)
 {
  memset (ButtonJoyGUID+Button,0,sizeof(GUID));
  m_ButJoy[Button]->SetCurSel(-1);
 }
 else
  memcpy (ButtonJoyGUID+Button,JoystickGUID+ButtonJoyIndex[Button],sizeof(GUID));
}

// Wrapper to write an integer to a .INI file
BOOL WritePrivateProfileInt (LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
 char	Buffer[16];

 sprintf (Buffer,"%d",Value);
 return WritePrivateProfileString(lpAppName,lpKeyName,Buffer,lpFileName);	
}

char *GUIDToString (GUID *G, char *Buffer)
{
 sprintf (Buffer,"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",G->Data1,G->Data2,G->Data3,G->Data4[0],G->Data4[1],G->Data4[2],G->Data4[3],G->Data4[4],G->Data4[5],G->Data4[6],G->Data4[7]);

 return Buffer;
}

GUID *StringToGUID (char *Buffer, GUID *G)
{
 WORD	Temp[8];

 sscanf (Buffer,"{%08lX-%04hX-%04hX-%02hX%02hX-%02hX%02hX%02hX%02hX%02hX%02hX}",&(G->Data1),&(G->Data2),&(G->Data3),Temp+0,Temp+1,Temp+2,Temp+3,Temp+4,Temp+5,Temp+6,Temp+7);

 G->Data4[0]= (BYTE) Temp[0];
 G->Data4[1]= (BYTE) Temp[1];
 G->Data4[2]= (BYTE) Temp[2];
 G->Data4[3]= (BYTE) Temp[3];
 G->Data4[4]= (BYTE) Temp[4];
 G->Data4[5]= (BYTE) Temp[5];
 G->Data4[6]= (BYTE) Temp[6];
 G->Data4[7]= (BYTE) Temp[7];

 return G;
}

void CPPJoyJoyDlg::SaveINI(char *Filename)
{
 int	Count;
 char	StringGUID[128];
 char	ValueName[128];

 WritePrivateProfileInt(PPJOYJOY_APPNAME,"VirtualJoyNumber",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1,Filename);

 for (Count=0;Count<8;Count++)
 {
  sprintf (ValueName,"Axis%dAxisIndex",Count+1);
  WritePrivateProfileInt(PPJOYJOY_APPNAME,ValueName,AxisAxisIndex[Count],Filename);

  sprintf (ValueName,"Axis%dJoyGUID",Count+1);
  WritePrivateProfileString (PPJOYJOY_APPNAME,ValueName,GUIDToString(AxisJoyGUID+Count,StringGUID),Filename);
 }

 for (Count=0;Count<16;Count++)
 {
  sprintf (ValueName,"Button%dButtonIndex",Count+1);
  WritePrivateProfileInt(PPJOYJOY_APPNAME,ValueName,ButtonButtonIndex[Count],Filename);

  sprintf (ValueName,"Button%dJoyGUID",Count+1);
  WritePrivateProfileString (PPJOYJOY_APPNAME,ValueName,GUIDToString(ButtonJoyGUID+Count,StringGUID),Filename);
 }

 WritePrivateProfileInt(PPJOYJOY_APPNAME,"UpdateInterval",UpdateInterval,Filename);
}

void CPPJoyJoyDlg::LoadINI(char *Filename)
{
 char	Buffer[1024];
 int	Count;
 char	StringGUID[128];
 char	ValueName[128];
 GUID	TempGUID;

 if (!GetPrivateProfileSectionNames (Buffer,sizeof(Buffer),Filename))
 {
  MessageBox ("Cannot read .ini file","Error",MB_OK);
  return;
 }

 SetComboSel(m_JoyCombo,GetPrivateProfileInt(PPJOYJOY_APPNAME,"VirtualJoyNumber",1,Filename)-1);
 OnSelchangeJoycombo();

 UpdateInterval= GetPrivateProfileInt(PPJOYJOY_APPNAME,"UpdateInterval",5,Filename);
 sprintf (Buffer,"%d",UpdateInterval);
 m_IntervalEdit.SetWindowText(Buffer);
 OnSetinterval();

 //!!! Update control for UpdateInterval

 for (Count=0;Count<8;Count++)
 {
  sprintf (ValueName,"Axis%dAxisIndex",Count+1);
  AxisAxisIndex[Count]= GetPrivateProfileInt(PPJOYJOY_APPNAME,ValueName,-1,Filename);

  sprintf (ValueName,"Axis%dJoyGUID",Count+1);
  GetPrivateProfileString (PPJOYJOY_APPNAME,ValueName,"{00000000-0000-0000-0000-000000000000}",StringGUID,sizeof(StringGUID),Filename);
  memcpy (AxisJoyGUID+Count,StringToGUID (StringGUID,&TempGUID),sizeof(TempGUID));
 }

 for (Count=0;Count<16;Count++)
 {
  sprintf (ValueName,"Button%dButtonIndex",Count+1);
  ButtonButtonIndex[Count]= GetPrivateProfileInt(PPJOYJOY_APPNAME,ValueName,-1,Filename);

  sprintf (ValueName,"Button%dJoyGUID",Count+1);
  GetPrivateProfileString (PPJOYJOY_APPNAME,ValueName,"{00000000-0000-0000-0000-000000000000}",StringGUID,sizeof(StringGUID),Filename);
  memcpy (ButtonJoyGUID+Count,StringToGUID (StringGUID,&TempGUID),sizeof(TempGUID));
 }

 OnRescan();
}


int CPPJoyJoyDlg::DoModal() 
{
 hTerminateEvent= CreateEvent(NULL,TRUE,FALSE,NULL);
 if (!hTerminateEvent)
 {
  MessageBox ("CreateEvent failed...");
  goto Exit;
 }

 CDialog::DoModal();

Exit:
 if (hTerminateEvent)
  CloseHandle(hTerminateEvent);
 hTerminateEvent= NULL;

 if (hVJoy!=INVALID_HANDLE_VALUE)
  CloseHandle (hVJoy);

 return IDCANCEL;
}

int CPPJoyJoyDlg::StartScanLoop(void)
{
 InitJoyState();

 ScanThread= NULL;

 ScanThread= AfxBeginThread(ScanLoop,NULL,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
 if (!ScanThread)
 {
  MessageBox ("AfxBeginThread failed...");
  return -3;
 }
 ScanThread->m_bAutoDelete= FALSE;
 ScanThread->ResumeThread();

 return 0;
}

void CPPJoyJoyDlg::StopScanLoop(void)
{
 if (ScanThread)
 {
  SetEvent (hTerminateEvent);

  WaitForSingleObject (ScanThread->m_hThread,INFINITE);
  delete ScanThread;

  ScanThread= NULL;

  ResetEvent (hTerminateEvent);
 }
}

int CPPJoyJoyDlg::PopulateJoyCombo()
{
#define	MAX_JOYSTICKS	16
 int		Count;
 int		NumJoysticks;
 JOYENTRY	JoystickList[MAX_JOYSTICKS];
 char		JoystickName[64];
 int		Index;

 // Open the PPJoy device driver. Return code 0= failure, 1= success.
 if (!PPJoy_OpenDriver())
  return 0;

 m_JoyCombo.ResetContent();

 NumJoysticks= PPJoy_GetJoystickList (JoystickList,MAX_JOYSTICKS);
 // If NumJoysticks<=0 for loop will just fall through
 for (Count=0;Count<NumJoysticks;Count++)
 {
  // Add any virtual joysticks we find to the list
  if (JoystickList[Count].JoyType==JOYTYPE_IOCTL)
  {
   // User friendly name of the joystick
   sprintf (JoystickName,"Virtual joystick %d",JoystickList[Count].UnitNumber+1);
   Index= m_JoyCombo.AddString(JoystickName);
   // And the internal PPJoy unit number for the joystick
   m_JoyCombo.SetItemData(Index,JoystickList[Count].UnitNumber);
  }
 }

 // Close the PPJoy device driver after we enumerated all the devices.
 PPJoy_CloseDriver();

 // Success if we found at least one virtual joystick
 return m_JoyCombo.GetCount()>0;
}

void CPPJoyJoyDlg::OnCancel() 
{
 if (MessageBox ("Are you sure you want to close PPJoyJoy?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)==IDYES)
  CDialog::OnCancel();
}
