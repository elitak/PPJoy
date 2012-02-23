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


// PPJoyMouseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyMouse.h"
#include "PPJoyMouseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// Include files for DirectInput and PPJoy IOCTL calls                     //
/////////////////////////////////////////////////////////////////////////////
#define	DIRECTINPUT_VERSION	0x0500
#include <dinput.h>
#include <winioctl.h>
#include <ppjioctl.h>

#include "afxwin.h"

#include "PPJoyAPI.h"
#include "Branding.h"

/////////////////////////////////////////////////////////////////////////////
// Define constants used in the application                                //
/////////////////////////////////////////////////////////////////////////////
#define	NUM_ANALOG	4					// Num analog values sent to PPJoy
#define	NUM_DIGITAL	16					// Num digital values sent to PPJoy
#define	PPJOYMOUSE_APPNAME	"PPJoyMouse"	// Section name for INI files

#define	WM_PPJOYSTATUS		WM_USER+1

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
LPDIRECTINPUT		di;					// Main DirectInput interface
LPDIRECTINPUTDEVICE	did;				// Interface to the mouse device
HANDLE				hEvent;				// Set by DirectInput on new input

int					TerminateFlag;		// If >0 scan thread terminates

										// PPJoy related fields
HANDLE				hVJoy;				// Handle for IOCTL to current joystick
char				VJoyName[256];		// Name of the current joystick device
JOYSTICK_STATE		js;					// Joystick state we will return

HWND				MainWindow;			// Window to receive status updates

char				PPJoyStatus[128];	// PPJoy status message
char				INIFilename[1024];	// Last INI file used (load or save)

DIMOUSESTATE		DIMS;				// DirectInput Mouse State

LONG				XGain;				// Multiplier for Mouse report - X Axis
LONG				YGain;				// Multiplier for Mouse report - Y Axis
LONG				ZGain;				// Multiplier for Mouse report - Z Axis

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
    m_ComponentName.SetWindowText ("PPJoyMouse Version " VER_PRODUCTVERSION_STR);
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
// CPPJoyMouseDlg dialog

CPPJoyMouseDlg::CPPJoyMouseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPPJoyMouseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPPJoyMouseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPPJoyMouseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPPJoyMouseDlg)
	DDX_Control(pDX, IDC_ZGAINEDIT, m_ZGainEdit);
	DDX_Control(pDX, IDC_YGAINEDIT, m_YGainEdit);
	DDX_Control(pDX, IDC_XGAINEDIT, m_XGainEdit);
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPPJoyMouseDlg, CDialog)
	//{{AFX_MSG_MAP(CPPJoyMouseDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	ON_EN_CHANGE(IDC_XGAINEDIT, OnChangeXgainedit)
	ON_EN_CHANGE(IDC_YGAINEDIT, OnChangeYgainedit)
	ON_EN_CHANGE(IDC_ZGAINEDIT, OnChangeZgainedit)
	ON_BN_CLICKED(IDC_LOADINI, OnLoadini)
	ON_BN_CLICKED(IDC_SAVEINI, OnSaveini)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPPJoyMouseDlg message handlers

BOOL CPPJoyMouseDlg::OnInitDialog()
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

 MainWindow= AfxGetMainWnd()->m_hWnd;

 m_XGainEdit.SetWindowText("75");
 m_YGainEdit.SetWindowText("75");
 m_ZGainEdit.SetWindowText("75");

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();

 char			*CmdLine;

 CmdLine= AfxGetApp()->m_lpCmdLine;
 if (*CmdLine)
  LoadINI (CmdLine);

 return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPPJoyMouseDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPPJoyMouseDlg::OnPaint() 
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
HCURSOR CPPJoyMouseDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

/////////////////////////////////////////////////////////

HRESULT ReadDirectInputData (void)
{
 HRESULT hr;
    
 if (!did)
  return S_OK;

 hr= did->GetDeviceState(sizeof(DIMS),&DIMS);
 if (FAILED(hr)) 
 {
  while(hr==DIERR_INPUTLOST) 
   hr= did->Acquire();

  return hr;
 }

 return S_OK;
}

/////////////////////////////////////////////////////////

int	UpdateVirtualJoyStatus;

void OpenPPJoyDevice (void)
{
 int	Count;

 UpdateVirtualJoyStatus= 1;

 hVJoy= CreateFile(VJoyName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL); 

 if (hVJoy==INVALID_HANDLE_VALUE)
 {
  sprintf (PPJoyStatus,"Error %d opening joystick",GetLastError());
  PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
  return;
 }

 sprintf (PPJoyStatus,"Successfully opened joystick");
 PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);

 // Initialise global members of joystick state
 memset (&js,0,sizeof(js));
 js.Signature= JOYSTICK_STATE_V1;
 js.NumAnalog= NUM_ANALOG;
 js.NumDigital= NUM_DIGITAL;

 // Initialise axis values to the middle of the range...
 for (Count=0;Count<NUM_ANALOG;Count++)
  js.Axis[Count]= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;
}

void UpdatePPJoy (void)
{
 int			Count;
 long			AxisValue;
 DWORD			RetSize;
 DWORD			rc;

 // Make sure that the dialog window is already displayed. Else we simply return...
 if (!MainWindow)
  return;

 if (hVJoy==INVALID_HANDLE_VALUE)
  OpenPPJoyDevice();

 if (hVJoy==INVALID_HANDLE_VALUE)
  return;

 AxisValue= js.Axis[0]+DIMS.lX*XGain;
 if (AxisValue<PPJOY_AXIS_MIN) AxisValue= PPJOY_AXIS_MIN;
 if (AxisValue>PPJOY_AXIS_MAX) AxisValue= PPJOY_AXIS_MAX;
 js.Axis[0]= AxisValue;

 AxisValue= js.Axis[1]+DIMS.lY*YGain;
 if (AxisValue<PPJOY_AXIS_MIN) AxisValue= PPJOY_AXIS_MIN;
 if (AxisValue>PPJOY_AXIS_MAX) AxisValue= PPJOY_AXIS_MAX;
 js.Axis[1]= AxisValue;

 AxisValue= js.Axis[2]+DIMS.lZ*ZGain;
 if (AxisValue<PPJOY_AXIS_MIN) AxisValue= PPJOY_AXIS_MIN;
 if (AxisValue>PPJOY_AXIS_MAX) AxisValue= PPJOY_AXIS_MAX;
 js.Axis[2]= AxisValue;

 for (Count=0;Count<4;Count++)
  js.Digital[Count]= (DIMS.rgbButtons[Count]&0x80)?1:0;

 if (!DeviceIoControl(hVJoy,IOCTL_PPORTJOY_SET_STATE,&js,sizeof(js),NULL,0,&RetSize,NULL))
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
  PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
 else if (UpdateVirtualJoyStatus)
 {
  UpdateVirtualJoyStatus= 0;
  sprintf (PPJoyStatus,"Sending joystick updates to PPJoy");
  PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
}

UINT ScanLoop (LPVOID pParam)
{
 while (1)
 {
  // Wait until a key is pressed or our loop terminated...
  WaitForSingleObject (hEvent,INFINITE);

  // Check to see if the terminate flag is set. If yes get out of here
  if (TerminateFlag)
   break;

  // Read mouse input. If reading fails go back to start of the loop
  if (ReadDirectInputData()!=S_OK)
   continue;

  // Update PPJoy virtual stick
  UpdatePPJoy();

//  if (DebugBeep)
//   Beep(5000,30);
 }

 return 0;
}

int CPPJoyMouseDlg::DoModal() 
{
 CWinThread		*pThread;

 *VJoyName= 0;
 hVJoy= INVALID_HANDLE_VALUE;

 MainWindow= NULL;	// Must sure the Window handle is NULL or valid...

 di= NULL;
 did= NULL;
 hEvent= NULL;
 pThread= NULL;

 hEvent= CreateEvent(NULL,FALSE,FALSE,NULL);
 if (!hEvent)
 {
  MessageBox ("CreateEvent failed...");
  goto Exit;
 }

 TerminateFlag= 0;
 pThread= AfxBeginThread(ScanLoop,NULL,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
 if (!pThread)
 {
  MessageBox ("AfxBeginThread failed...");
  goto Exit;
 }
 pThread->m_bAutoDelete= FALSE;
 pThread->ResumeThread();

 CDialog::DoModal();

 TerminateFlag= 1;
 SetEvent (hEvent);

 WaitForSingleObject (pThread->m_hThread,INFINITE);
 delete pThread;

Exit:
 pThread= NULL;

 if (hEvent)
  CloseHandle(hEvent);
 hEvent= NULL;

 if (hVJoy!=INVALID_HANDLE_VALUE)
  CloseHandle (hVJoy);

 return IDCANCEL;
}

void CPPJoyMouseDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\" JOY_IOCTL_DEV_NAME,m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"PPJoy Mouse to Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);
}

LRESULT CPPJoyMouseDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
{
 m_PPJoyStatus.SetWindowText (PPJoyStatus);
 return 0;
}

void CPPJoyMouseDlg::OnChangeXgainedit() 
{
 char	Value[256];

 m_XGainEdit.GetWindowText (Value,sizeof(Value));
 XGain= atoi(Value);
}

void CPPJoyMouseDlg::OnChangeYgainedit() 
{
 char	Value[256];

 m_YGainEdit.GetWindowText (Value,sizeof(Value));
 YGain= atoi(Value);
}

void CPPJoyMouseDlg::OnChangeZgainedit() 
{
 char	Value[256];

 m_ZGainEdit.GetWindowText (Value,sizeof(Value));
 ZGain= atoi(Value);
}

// Display file-open dialog. If the user picks an INI file load it.
void CPPJoyMouseDlg::OnLoadini() 
{
 CFileDialog	fd(TRUE);

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 fd.m_ofn.Flags|= OFN_FILEMUSTEXIST;

 if (fd.DoModal()==IDOK)
  LoadINI (fd.m_ofn.lpstrFile);
}

// Display file-save dialog. If the user picks an INI file save it.
void CPPJoyMouseDlg::OnSaveini() 
{
 CFileDialog	fd(FALSE,".ini");

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 if (fd.DoModal()==IDOK)
  SaveINI (fd.m_ofn.lpstrFile);
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

void CPPJoyMouseDlg::LoadINI (char *Filename)
{
 char			Buffer[1024];

 if (!GetPrivateProfileSectionNames (Buffer,sizeof(Buffer),Filename))
 {
  MessageBox ("Cannot read .ini file","Error",MB_OK);
  return;
 }

// m_DebugBeep.SetCheck(GetPrivateProfileInt(PPJOYMOUSE_APPNAME,"DebugBeep",0,Filename));
// OnDebugbeep();

 SetComboSel(m_JoyCombo,GetPrivateProfileInt(PPJOYMOUSE_APPNAME,"VirtualJoyNumber",1,Filename)-1);
 OnSelchangeJoycombo();

 XGain= GetPrivateProfileInt(PPJOYMOUSE_APPNAME,"XGain",256,Filename);
 YGain= GetPrivateProfileInt(PPJOYMOUSE_APPNAME,"YGain",256,Filename);
 ZGain= GetPrivateProfileInt(PPJOYMOUSE_APPNAME,"ZGain",256,Filename);

 sprintf (Buffer,"%d",XGain);
 m_XGainEdit.SetWindowText (Buffer);
 sprintf (Buffer,"%d",YGain);
 m_YGainEdit.SetWindowText (Buffer);
 sprintf (Buffer,"%d",ZGain);
 m_ZGainEdit.SetWindowText (Buffer);
}

// Wrapper to write an integer to a .INI file
BOOL WritePrivateProfileInt (LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
 char	Buffer[16];

 sprintf (Buffer,"%d",Value);
 return WritePrivateProfileString(lpAppName,lpKeyName,Buffer,lpFileName);	
}

void CPPJoyMouseDlg::SaveINI (char *Filename)
{
// WritePrivateProfileInt(PPJOYMOUSE_APPNAME,"DebugBeep",m_DebugBeep.GetCheck(),Filename);
 WritePrivateProfileInt(PPJOYMOUSE_APPNAME,"VirtualJoyNumber",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1,Filename);

 WritePrivateProfileInt(PPJOYMOUSE_APPNAME,"XGain",XGain,Filename);
 WritePrivateProfileInt(PPJOYMOUSE_APPNAME,"YGain",YGain,Filename);
 WritePrivateProfileInt(PPJOYMOUSE_APPNAME,"ZGain",ZGain,Filename);
}

int CPPJoyMouseDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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

 hr= di->CreateDevice(GUID_SysMouse,&did,NULL);
 if (FAILED(hr))
 {
  MessageBox ("Error creating DirectInputDevice interface for the mouse");
  goto Exit;
 }

 hr= did->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd,DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
 if (FAILED(hr))
 {
  MessageBox ("Cannot set Cooperation level...");
  goto Exit;
 }

 hr= did->SetDataFormat(&c_dfDIMouse); 
 if (FAILED(hr))
 { 
  MessageBox ("Cannot set mouse data format...");
  goto Exit;
 }

 hr= did->SetEventNotification (hEvent);
 if (hr!=DI_OK)
 { 
  MessageBox ("SetNotification failed...");
  goto Exit;
 }

 hr= did->Acquire(); 
 if (FAILED(hr))
 { 
  MessageBox ("Cannot acquire mouse...");
  goto Exit;
 }

 return 0;

Exit:
 if (did)
  did->Release();
 did= NULL;

 if (di)
  di->Release();
 di= NULL;

 return -1;
}

void CPPJoyMouseDlg::OnDestroy() 
{
 if (did)
  did->Release();
 did= NULL;

 if (di)
  di->Release();
 di= NULL;

 CDialog::OnDestroy();
}

int CPPJoyMouseDlg::PopulateJoyCombo()
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

void CPPJoyMouseDlg::OnCancel() 
{
 if (MessageBox ("Are you sure you want to close PPJoyMouse?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)!=IDYES)
  return;

 // We unacquire the mouse device here because the window handle is destroyed before
 // DoModal returns. DirectInput requires the window handle passed to Acquire() to remain
 // valid while the device (mouse) is acquired.

 // Might as well also clear the event notification while we are at it...
 if (did)
 {
  did->Unacquire();
  did->SetEventNotification (NULL);
 }

 CDialog::OnCancel();
}

