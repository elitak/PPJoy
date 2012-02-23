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


// PPJoyDLLDlg.cpp : implementation file
//

#include "stdafx.h"
#include <mmsystem.h>
#include "PPJoyDLL.h"
#include "PPJoyDLLDlg.h"

/////////////////////////////////////////////////////////////////////////////
// Include files for PPJoy IOCTL calls                                     //
/////////////////////////////////////////////////////////////////////////////
#include <winioctl.h>
#include <ppjioctl.h>

#include "afxwin.h"

#include "PPJoyAPI.h"
#include "Branding.h"

/////////////////////////////////////////////////////////////////////////////
// Define constants used in the application                                //
/////////////////////////////////////////////////////////////////////////////
#define	NUM_ANALOG	16					// Num analog values sent to PPJoy
#define	NUM_DIGITAL	16					// Num digital values sent to PPJoy
#define	PPJOYDLL_APPNAME	"PPJoyDLL"	// Section name for INI files

#define	NUM_PROTOCOLS		2

#define	WM_PPJOYSTATUS		WM_USER+1
#define	WM_DLLSTATUS		WM_USER+2

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

struct DLL_PROTOCOL
{
 char	*Name;
 int (*Init) (void);
 int (*ReadJoy) (void);
};


/////////////////////////////////////////////////////////////////////////////
// Global variables used through-out the application                       //
/////////////////////////////////////////////////////////////////////////////

HANDLE				hTerminateEvent;	// Signal scan thread to exit
CWinThread			*ScanThread;		// Handle to scan loop thread

int					DebugBeep;			// If >0 Beep on serial input

HANDLE				hVJoy= INVALID_HANDLE_VALUE;
										// Handle for IOCTL to current joystick
char				VJoyName[64];		// Name of the current joystick device
JOYSTICK_STATE		JoyState;			// Joystick data to send to PPJoy;

///char				InputDLLName[256];	// Name of the DLL used to read the joystick
HINSTANCE			hInputDLL= NULL;	// Handle of the input DLL

DWORD				UpdateInterval;		// Delay in ms between reading joystick updates
int					ActiveProtocol= -1;	// Index of protocol currently selected
HWND				MainWindow;			// Window to receive status updates

char				INIFilename[1024];	// Last INI file used (load or save)
char				PPJoyStatus[128];	// PPJoy status message
char				DLLStatus[128];		// Serial (port) status message

int InitSmartpropo (void);
int ReadSmartpropo (void);
int InitAsyncDLL (void);
int CleanupAsyncDLL (void);
int AsyncDLLCallback (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital);
void UpdatePPJoy (void);


DLL_PROTOCOL		DLLProtocol[NUM_PROTOCOLS]=
					{
					{"SmartPropo winmm.dll",InitSmartpropo,ReadSmartpropo},
					{"Callback DLL interface",InitAsyncDLL,NULL}
					};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int (_cdecl *DLLCleanupFunction) (void);

typedef MMRESULT (__stdcall *SmartProto_joyGetPosEx_Proto) (UINT uJoyID, LPJOYINFOEX pji);
typedef MMRESULT (__stdcall *SmartProto_joyGetDevCapsA_Proto) (UINT uJoyID, LPJOYCAPS pjc, UINT cbjc);

typedef int (_cdecl AsyncDLL_Callback_Proto) (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital);

typedef int (__stdcall *AsyncDLL_InitDLL_Proto) (AsyncDLL_Callback_Proto CallbackFunc);
typedef int (__stdcall *AsyncDLL_Cleanup_Proto) (void);


SmartProto_joyGetPosEx_Proto	SmartProto_joyGetPosEx;
SmartProto_joyGetDevCapsA_Proto	SmartProto_joyGetDevCapsA;

AsyncDLL_InitDLL_Proto			AsyncDLL_InitDLL;
AsyncDLL_Cleanup_Proto			AsyncDLL_Cleanup;

int InitSmartpropo (void)
{
 SmartProto_joyGetPosEx= (SmartProto_joyGetPosEx_Proto) GetProcAddress(hInputDLL,"joyGetPosEx");
 if (!SmartProto_joyGetPosEx)
 {
  sprintf (DLLStatus,"Error %d getting joyGetPosEx() entry",GetLastError());
  PostMessage (MainWindow,WM_DLLSTATUS,0,(long)DLLStatus);
  return 0;
 }


 SmartProto_joyGetDevCapsA= (SmartProto_joyGetDevCapsA_Proto) GetProcAddress(hInputDLL,"joyGetDevCapsA");
 if (!SmartProto_joyGetDevCapsA)
 {
  sprintf (DLLStatus,"Error %d getting joyGetDevCapsA() entry",GetLastError());
  PostMessage (MainWindow,WM_DLLSTATUS,0,(long)DLLStatus);
  return 0;
 }

 return 1;
}

DWORD ScaleAxisValues (DWORD Position, DWORD InMin, DWORD InMax, DWORD OutMin, DWORD OutMax)
{
 return (Position-InMin)*(OutMax-OutMin)/(InMax-InMin)+OutMin;
}

int ReadSmartpropo (void)
{
 JOYINFOEX	JI;
 JOYCAPS	JC;
 MMRESULT	rc;

 rc= SmartProto_joyGetDevCapsA (0,&JC,sizeof(JC));
 if (rc!=JOYERR_NOERROR)
  return 0;

 JI.dwSize= sizeof(JI);
 JI.dwFlags= JOY_RETURNALL;
 rc= SmartProto_joyGetPosEx(0,&JI);
 
 if (rc!=JOYERR_NOERROR)
  return 0;

 JoyState.Axis[0]= ScaleAxisValues (JI.dwXpos,JC.wXmin,JC.wXmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
 JoyState.Axis[1]= ScaleAxisValues (JI.dwYpos,JC.wYmin,JC.wYmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
 JoyState.Axis[2]= ScaleAxisValues (JI.dwZpos,JC.wZmin,JC.wZmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
 JoyState.Axis[3]= ScaleAxisValues (JI.dwRpos,JC.wRmin,JC.wRmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
 JoyState.Axis[4]= ScaleAxisValues (JI.dwUpos,JC.wUmin,JC.wUmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
 JoyState.Axis[5]= ScaleAxisValues (JI.dwVpos,JC.wVmin,JC.wVmax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);
	  
 return 1;
}

int AsyncDLLCallback (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital)
{
 int	Count;

 if (NumAnalog>NUM_ANALOG) NumAnalog= NUM_ANALOG;
 if (NumDigital>NUM_DIGITAL) NumDigital= NUM_DIGITAL;

 for (Count=0;Count<NumAnalog;Count++)
  JoyState.Axis[Count]= ScaleAxisValues (Analog[Count],AnalogMin,AnalogMax,PPJOY_AXIS_MIN,PPJOY_AXIS_MAX);

 for (Count=0;Count<NumDigital;Count++)
  JoyState.Digital[Count]= Digital[Count]?1:0;

 UpdatePPJoy();
 if (DebugBeep)
  Beep(5000,1);

 return 1;
}

int InitAsyncDLL (void)
{
 AsyncDLL_InitDLL= (AsyncDLL_InitDLL_Proto) GetProcAddress(hInputDLL,"InitDLL");
 if (!AsyncDLL_InitDLL)
 {
  sprintf (DLLStatus,"Error %d getting InitDLL() entry",GetLastError());
  PostMessage (MainWindow,WM_DLLSTATUS,0,(long)DLLStatus);
  return 0;
 }

 AsyncDLL_Cleanup= (AsyncDLL_Cleanup_Proto) GetProcAddress(hInputDLL,"Cleanup");
 if (!AsyncDLL_Cleanup)
 {
  sprintf (DLLStatus,"Error %d getting Cleanup() entry",GetLastError());
  PostMessage (MainWindow,WM_DLLSTATUS,0,(long)DLLStatus);
  return 0;
 }

 DLLCleanupFunction= CleanupAsyncDLL;

 return AsyncDLL_InitDLL(AsyncDLLCallback);
}

int CleanupAsyncDLL (void)
{
 // This causes an access violation when the protocol is changed and a new DLL is then loaded. Do Fix!

 if (AsyncDLL_Cleanup)
  return AsyncDLL_Cleanup();

 return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Utilities to help hande .INI files
//////////////////////////////////////////////////////////////////////////////

// Wrapper to write an integer to a .INI file
BOOL WritePrivateProfileInt (LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
 char	Buffer[16];

 sprintf (Buffer,"%d",Value);
 return WritePrivateProfileString(lpAppName,lpKeyName,Buffer,lpFileName);	
}

//////////////////////////////////////////////////////////////////////////////
// Utilities to help ComboBox operations
//////////////////////////////////////////////////////////////////////////////

void AddComboItem (CComboBox &ComboBox, char *Name, DWORD Value)
{
 int	Index;

 Index= ComboBox.AddString(Name);
 if (Index>=0)
  ComboBox.SetItemData (Index,Value);
}

void SetComboSel (CComboBox &ComboBox, int DesiredItemData)
{
 int	Count;

 Count= ComboBox.GetCount();
 while (Count--)
  if (ComboBox.GetItemData(Count)==(DWORD)DesiredItemData)
   break;
 ComboBox.SetCurSel(Count);
}

char* GetComboSel (CComboBox &ComboBox, char *Buffer, int BufSize)
{
 int	Index;

 *Buffer= 0;
 Index= ComboBox.GetCurSel();
 if (Index!=CB_ERR)
  ComboBox.GetWindowText(Buffer,BufSize);

 return Buffer;
}

int GetComboSel (CComboBox &ComboBox)
{
 DWORD	ItemData;
 int	Index;

 Index= ComboBox.GetCurSel();
 if (Index==CB_ERR)
  return -1;

 ItemData= ComboBox.GetItemData(Index);
 if (ItemData==LB_ERR)
  return -1;
 
 return ItemData;
}

//////////////////////////////////////////////////////////////////////////////
// PPJoy interface routines
//////////////////////////////////////////////////////////////////////////////

int	UpdateVirtualJoyStatus;

void OpenPPJoyDevice (void)
{
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
 if (!MainWindow)
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
  PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
 else if (UpdateVirtualJoyStatus)
 {
  UpdateVirtualJoyStatus= 0;
  sprintf (PPJoyStatus,"Sending joystick updates to PPJoy");
  PostMessage (MainWindow,WM_PPJOYSTATUS,0,(long)PPJoyStatus);
 }
}

//////////////////////////////////////////////////////////////////////////////
// DLL reading and update routines
//////////////////////////////////////////////////////////////////////////////

UINT ScanLoop (LPVOID pParam)
{
 char		UpdateJoystick;

 while (WaitForSingleObject(hTerminateEvent,UpdateInterval)==WAIT_TIMEOUT)
 {
  if ((ActiveProtocol<0)||(ActiveProtocol>=NUM_PROTOCOLS))
   continue;

  UpdateJoystick= DLLProtocol[ActiveProtocol].ReadJoy();

  // Update PPJoy virtual stick
  if (UpdateJoystick)
  {
   UpdatePPJoy();
   if (DebugBeep)
    Beep(5000,1);
  }
 }

 return 0;
}

int CPPJoyDLLDlg::StartScanLoop()
{
 char			DLLName[256];
 char			Buffer[256];

 m_DLLNameEdit.GetWindowText(DLLName,sizeof(DLLName));
 if (!*DLLName)
 {
//  MessageBox ("No .DLL specified, not starting the scan thread...");
  return -4;
 }

 hInputDLL= LoadLibrary (DLLName);
 if (!hInputDLL)
 {
  sprintf (Buffer,"LoadLibrary error %d trying to load .DLL",GetLastError());
  m_DLLStatus.SetWindowText(Buffer);
  return -5;
 }

 m_DLLStatus.SetWindowText("Loaded .DLL");
 ResetEvent (hTerminateEvent);

 if ((ActiveProtocol<0)||(ActiveProtocol>=NUM_PROTOCOLS))
  return -1;

 if (!DLLProtocol[ActiveProtocol].Init())
  return -2;

 InitJoyState();

 ScanThread= NULL;
 if (DLLProtocol[ActiveProtocol].ReadJoy)		// Do we need a scan thread for this interface?
 {
  ScanThread= AfxBeginThread(ScanLoop,NULL,THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
  if (!ScanThread)
  {
   MessageBox ("AfxBeginThread failed...");
   return -3;
  }
  ScanThread->m_bAutoDelete= FALSE;
  ScanThread->ResumeThread();
 }

 return 0;
}

void CPPJoyDLLDlg::StopScanLoop()
{
 if ((ActiveProtocol<0)||(ActiveProtocol>=NUM_PROTOCOLS))
  return;
 
 if (DLLCleanupFunction)
 {
  DLLCleanupFunction();
  DLLCleanupFunction= NULL;
 }

 if (ScanThread)
 {
  SetEvent (hTerminateEvent);

  WaitForSingleObject (ScanThread->m_hThread,INFINITE);
  delete ScanThread;

  ScanThread= NULL;
 }

 if (hInputDLL)
 {
  FreeLibrary (hInputDLL);
  hInputDLL= NULL;
 }
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
    m_ComponentName.SetWindowText ("PPJoyDLL Version " VER_PRODUCTVERSION_STR);
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
// CPPJoyDLLDlg dialog

CPPJoyDLLDlg::CPPJoyDLLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPPJoyDLLDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPPJoyDLLDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPPJoyDLLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPPJoyDLLDlg)
	DDX_Control(pDX, IDC_INTERVALTEXT2, m_IntervalText2);
	DDX_Control(pDX, IDC_INTERVALTEXT1, m_IntervalText1);
	DDX_Control(pDX, IDC_SETINTERVAL, m_SetIntervalButton);
	DDX_Control(pDX, IDC_DLLNAMEEDIT, m_DLLNameEdit);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	DDX_Control(pDX, IDC_INTERVALEDIT, m_UpdateInterval);
	DDX_Control(pDX, IDC_DLLSTATUS, m_DLLStatus);
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	DDX_Control(pDX, IDC_DEBUGBEEP, m_DebugBeep);
	DDX_Control(pDX, IDC_DLLTYPECOMBO, m_DLLTypeCombo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPPJoyDLLDlg, CDialog)
	//{{AFX_MSG_MAP(CPPJoyDLLDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
    ON_MESSAGE(WM_DLLSTATUS, OnDLLStatus)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOADDLL, OnLoaddll)
	ON_BN_CLICKED(IDC_LOADINI, OnLoadini)
	ON_BN_CLICKED(IDC_SAVEINI, OnSaveini)
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	ON_CBN_SELCHANGE(IDC_DLLTYPECOMBO, OnSelchangeDlltypecombo)
	ON_BN_CLICKED(IDC_SETINTERVAL, OnSetinterval)
	ON_BN_CLICKED(IDC_DEBUGBEEP, OnDebugbeep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPPJoyDLLDlg message handlers

BOOL CPPJoyDLLDlg::OnInitDialog()
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

 LoadComboValues();

 m_UpdateInterval.SetWindowText("25");
 OnSetinterval();
	
 m_DLLTypeCombo.SetCurSel(0);
 OnSelchangeDlltypecombo();

 m_DebugBeep.SetCheck(0);
 OnDebugbeep();

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();
 
 char			*CmdLine;

 CmdLine= AfxGetApp()->m_lpCmdLine;
 if (*CmdLine)
  LoadINI (CmdLine);
 
 EnableControls();

 StartScanLoop();

 return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPPJoyDLLDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPPJoyDLLDlg::OnPaint() 
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
HCURSOR CPPJoyDLLDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPPJoyDLLDlg::OnLoaddll() 
{
 CFileDialog	fd(TRUE);
 char			DLLName[256];

 StopScanLoop();

 m_DLLNameEdit.GetWindowText(DLLName,sizeof(DLLName));

 fd.m_ofn.lpstrFilter= "DLL Files\0*.dll\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= DLLName;
 fd.m_ofn.nMaxFile= sizeof(DLLName);

 fd.m_ofn.Flags|= OFN_FILEMUSTEXIST;

 if (fd.DoModal()!=IDOK)
  return;

 m_DLLNameEdit.SetWindowText(DLLName);

 StartScanLoop();
}

void CPPJoyDLLDlg::OnLoadini() 
{
 CFileDialog	fd(TRUE);

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 fd.m_ofn.Flags|= OFN_FILEMUSTEXIST;

 if (fd.DoModal()==IDOK)
 {
  LoadINI (fd.m_ofn.lpstrFile);
  StartScanLoop();
 }
}

void CPPJoyDLLDlg::OnSaveini() 
{
 CFileDialog	fd(FALSE,".ini");

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 if (fd.DoModal()==IDOK)
  SaveINI (fd.m_ofn.lpstrFile);
}

void CPPJoyDLLDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\" JOY_IOCTL_DEV_NAME,m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"PPJoy Input DLL to Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);
}

void CPPJoyDLLDlg::OnSelchangeDlltypecombo() 
{
 StopScanLoop();

 ActiveProtocol= m_DLLTypeCombo.GetCurSel();

 EnableControls();

 m_DLLStatus.SetWindowText("No .DLL specified");
 m_DLLNameEdit.SetWindowText ("");
}

void CPPJoyDLLDlg::OnSetinterval() 
{
 char	Buffer[64];
 DWORD	Interval;

 m_UpdateInterval.GetWindowText (Buffer,sizeof(Buffer));

 Interval= atoi (Buffer);
 if (Interval<5)
 {
  Interval=5;
  sprintf (Buffer,"%d",Interval);
  m_UpdateInterval.SetWindowText (Buffer);
 }

 UpdateInterval= Interval;
}

LRESULT CPPJoyDLLDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
{
 m_PPJoyStatus.SetWindowText (PPJoyStatus);
 return 0;
}

LRESULT CPPJoyDLLDlg::OnDLLStatus(WPARAM wParam, LPARAM lParam)
{
 m_DLLStatus.SetWindowText (DLLStatus);
 return 0;
}

void CPPJoyDLLDlg::OnDebugbeep() 
{
 DebugBeep= m_DebugBeep.GetCheck();
}

int CPPJoyDLLDlg::DoModal() 
{
 hTerminateEvent= CreateEvent(NULL,TRUE,FALSE,NULL);
 if (!hTerminateEvent)
 {
  MessageBox ("CreateEvent failed...");
  goto Exit;
 }

 CDialog::DoModal();

 StopScanLoop();

Exit:
 if (hTerminateEvent)
  CloseHandle(hTerminateEvent);
 hTerminateEvent= NULL;

 if (hVJoy!=INVALID_HANDLE_VALUE)
  CloseHandle (hVJoy);

 return IDCANCEL;
}

void CPPJoyDLLDlg::SaveINI(char *Filename)
{
 char			DLLName[256];

 m_DLLNameEdit.GetWindowText(DLLName,sizeof(DLLName));

 WritePrivateProfileInt(PPJOYDLL_APPNAME,"DebugBeep",m_DebugBeep.GetCheck(),Filename);
 WritePrivateProfileInt(PPJOYDLL_APPNAME,"VirtualJoyNumber",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1,Filename);

 WritePrivateProfileString(PPJOYDLL_APPNAME,"InputDLLName",DLLName,Filename);
 WritePrivateProfileInt(PPJOYDLL_APPNAME,"InputDLLType",m_DLLTypeCombo.GetCurSel(),Filename);
 WritePrivateProfileInt(PPJOYDLL_APPNAME,"UpdateInterval",UpdateInterval,Filename);
}

void CPPJoyDLLDlg::LoadINI(char *Filename)
{
 char			Buffer[1024];
 char			DLLName[256];

 if (!GetPrivateProfileSectionNames (Buffer,sizeof(Buffer),Filename))
 {
  MessageBox ("Cannot read .ini file","Error",MB_OK);
  return;
 }

 m_DebugBeep.SetCheck(GetPrivateProfileInt(PPJOYDLL_APPNAME,"DebugBeep",0,Filename));
 OnDebugbeep();

 SetComboSel(m_JoyCombo,GetPrivateProfileInt(PPJOYDLL_APPNAME,"VirtualJoyNumber",1,Filename)-1);
 OnSelchangeJoycombo();

 ActiveProtocol= GetPrivateProfileInt(PPJOYDLL_APPNAME,"InputDLLType",0,Filename);
 m_DLLTypeCombo.SetCurSel(ActiveProtocol);

 GetPrivateProfileString(PPJOYDLL_APPNAME,"InputDLLName","",DLLName,sizeof(DLLName),Filename);
 m_DLLNameEdit.SetWindowText(DLLName);

 UpdateInterval= GetPrivateProfileInt(PPJOYDLL_APPNAME,"UpdateInterval",0,Filename);
 sprintf (Buffer,"%d",UpdateInterval);
 m_UpdateInterval.SetWindowText (Buffer);
}

void CPPJoyDLLDlg::LoadComboValues()
{
 int	Count;

 m_DLLTypeCombo.ResetContent();
 for (Count=0;Count<NUM_PROTOCOLS;Count++)
  m_DLLTypeCombo.AddString(DLLProtocol[Count].Name);
}

void CPPJoyDLLDlg::EnableControls()
{
 switch (ActiveProtocol)
 {
  case 1:	m_SetIntervalButton.EnableWindow (FALSE);
			m_UpdateInterval.EnableWindow(FALSE);
			m_IntervalText1.EnableWindow(FALSE);
			m_IntervalText2.EnableWindow(FALSE);
			break;
  default:
  case 0:	m_SetIntervalButton.EnableWindow (TRUE);
			m_UpdateInterval.EnableWindow(TRUE);
			m_IntervalText1.EnableWindow(TRUE);
			m_IntervalText2.EnableWindow(TRUE);
			break;
 }
}

int CPPJoyDLLDlg::PopulateJoyCombo()
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

void CPPJoyDLLDlg::OnCancel() 
{
 if (MessageBox ("Are you sure you want to close PPJoyDLL?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)==IDYES)
  CDialog::OnCancel();
}

