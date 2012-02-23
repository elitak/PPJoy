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


// PPJoyKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyKey.h"
#include "PPJoyKeyDlg.h"

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
#define	PPJOYKEY_APPNAME	"PPJoyKey"	// Section name for INI files

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

struct SCANCODE
{
 int	Scancode;
 char	*Name;
};

/////////////////////////////////////////////////////////////////////////////
// Global variables used through-out the application                       //
/////////////////////////////////////////////////////////////////////////////

										// DirectInput related:
LPDIRECTINPUT		di;					// Main DirectInput interface
LPDIRECTINPUTDEVICE	did;				// Interface to keybaord device
BYTE				DIKS[256];			// Buffer for current keyboard input
HANDLE				hEvent;				// Set by DirectInput on new input

int					TerminateFlag;		// If >0 scan thread terminates
int					DebugBeep;			// If >0 Beep on each keypress
										// Scan codes to match for joystick
int					AxisMinIdx[NUM_ANALOG];
int					AxisMaxIdx[NUM_ANALOG];
int					ButtonIdx[NUM_DIGITAL];

										// PPJoy related fields
HANDLE				hVJoy;				// Handle for IOCTL to current joystick
char				VJoyName[256];		// Name of the current joystick device

HWND				MainWindow;			// Window to receive status updates

char				INIFilename[1024];	// Last INI file used (load or save)
char				PPJoyStatus[128];	// PPJoy status message

SCANCODE			Scancodes[]=
					{{-1,"No Key"},
					{DIK_ESCAPE,"Escape"},
					{DIK_F1,"F1"},
					{DIK_F2,"F2"},
					{DIK_F3,"F3"},
					{DIK_F4,"F4"},
					{DIK_F5,"F5"},
					{DIK_F6,"F6"},
					{DIK_F7,"F7"},
					{DIK_F8,"F8"},
					{DIK_F9,"F9"},
					{DIK_F10,"F10"},
					{DIK_F11,"F11"},
					{DIK_F12,"F12"},

					{DIK_GRAVE,"`"},
					{DIK_1,"1"},
					{DIK_2,"2"},
					{DIK_3,"3"},
					{DIK_4,"4"},
					{DIK_5,"5"},
					{DIK_6,"6"},
					{DIK_7,"7"},
					{DIK_8,"8"},
					{DIK_9,"9"},
					{DIK_0,"0"},
					{DIK_MINUS,"-"},
					{DIK_EQUALS,"="},
					{DIK_BACK,"Backspace"},

					{DIK_TAB,"Tab"},
					{DIK_Q,"Q"},
					{DIK_W,"W"},
					{DIK_E,"E"},
					{DIK_R,"R"},
					{DIK_T,"T"},
					{DIK_Y,"Y"},
					{DIK_U,"U"},
					{DIK_I,"I"},
					{DIK_O,"O"},
					{DIK_P,"P"},
					{DIK_LBRACKET,"["},
					{DIK_RBRACKET,"]"},
					{DIK_BACKSLASH,"\\"},

					{DIK_CAPITAL,"Caps Lock"},
					{DIK_A,"A"},
					{DIK_S,"S"},
					{DIK_D,"D"},
					{DIK_F,"F"},
					{DIK_G,"G"},
					{DIK_H,"H"},
					{DIK_J,"J"},
					{DIK_K,"K"},
					{DIK_L,"L"},
					{DIK_SEMICOLON,";"},
					{DIK_APOSTROPHE,"'"},
					{DIK_RETURN,"Enter"},

					{DIK_LSHIFT,"Left Shift"},
					{DIK_Z,"Z"},
					{DIK_X,"X"},
					{DIK_C,"C"},
					{DIK_V,"V"},
					{DIK_B,"B"},
					{DIK_N,"N"},
					{DIK_M,"M"},
					{DIK_COMMA,","},
					{DIK_PERIOD,"."},
					{DIK_SLASH,"/"},
					{DIK_RSHIFT,"Right Shift"},

					{DIK_LCONTROL,"Left Ctrl"},
					{DIK_LWIN,"Left Windows"},
					{DIK_LMENU,"Left Alt"},
					{DIK_SPACE,"Spacebar"},
					{DIK_RMENU,"Right Alt"},
					{DIK_RWIN,"Right Windows"},
					{DIK_APPS,"AppMenu"},
					{DIK_RCONTROL,"Right Ctrl"},

					{DIK_SYSRQ,"PrtScrn/SysRq"},
					{DIK_SCROLL,"Scroll Lock"},
					{DIK_PAUSE,"Pause/Break"},

					{DIK_INSERT,"Insert"},
					{DIK_HOME,"Home"},
					{DIK_PRIOR,"PgUp"},
					{DIK_DELETE,"Delete"},
					{DIK_END,"End"},
					{DIK_NEXT,"PgDn"},

					{DIK_UP,"Up"},
					{DIK_LEFT,"Left"},
					{DIK_DOWN,"Down"},
					{DIK_RIGHT,"Right"},

					{DIK_NUMLOCK,"Num Lock"},
					{DIK_DIVIDE,"Numpad /"},
					{DIK_SUBTRACT,"Numpad -"},
					{DIK_MULTIPLY,"Numpad *"},
					{DIK_ADD,"Numpad +"},
					{DIK_NUMPADENTER,"Numpad Enter"},
					{DIK_NUMPAD7,"Numpad 7"},
					{DIK_NUMPAD8,"Numpad 8"},
					{DIK_NUMPAD9,"Numpad 9"},
					{DIK_NUMPAD4,"Numpad 4"},
					{DIK_NUMPAD5,"Numpad 5"},
					{DIK_NUMPAD6,"Numpad 6"},
					{DIK_NUMPAD1,"Numpad 1"},
					{DIK_NUMPAD2,"Numpad 2"},
					{DIK_NUMPAD3,"Numpad 3"},
					{DIK_NUMPAD0,"Numpad 0"},
					{DIK_DECIMAL,"Numpad ."},

					{DIK_POWER,"Power"},
					{DIK_SLEEP,"Sleep"},

					{DIK_AT,"PC98 @"},
					{DIK_COLON,"PC98 :"},
					{DIK_UNDERLINE,"PC98 _"},
					{DIK_F13,"PC98 F13"},
					{DIK_F14,"PC98 F14"},
					{DIK_F15,"PC98 F15"},
					{DIK_NUMPADEQUALS,"PC98 Nump ="},
					{DIK_NUMPADCOMMA,"PC98 Nump ,"},
					{DIK_STOP,"PC98 Stop"},

					{DIK_AX,"Jp AX"},
					{DIK_CIRCUMFLEX,"Jp Circumflex"},
					{DIK_CONVERT,"Jp Convert"},
					{DIK_KANA,"Jp Kana"},
					{DIK_KANJI,"Jp Kanji"},
					{DIK_NOCONVERT,"Jp NoConvert"},
					{DIK_UNLABELED,"Jp Unlabeled"},
					{DIK_YEN,"Jp Yen"},
					{0,NULL}};

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
    m_ComponentName.SetWindowText ("PPJoyKey Version " VER_PRODUCTVERSION_STR);
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
// CPPJoyKeyDlg dialog

CPPJoyKeyDlg::CPPJoyKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPPJoyKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPPJoyKeyDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPPJoyKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPPJoyKeyDlg)
	DDX_Control(pDX, IDC_BUT16COMBO, m_But16Combo);
	DDX_Control(pDX, IDC_BUT15COMBO, m_But15Combo);
	DDX_Control(pDX, IDC_BUT14COMBO, m_But14Combo);
	DDX_Control(pDX, IDC_BUT13COMBO, m_But13Combo);
	DDX_Control(pDX, IDC_BUT12COMBO, m_But12Combo);
	DDX_Control(pDX, IDC_BUT11COMBO, m_But11Combo);
	DDX_Control(pDX, IDC_BUT10COMBO, m_But10Combo);
	DDX_Control(pDX, IDC_BUT9COMBO, m_But9Combo);
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	DDX_Control(pDX, IDC_BUT8COMBO, m_But8Combo);
	DDX_Control(pDX, IDC_BUT7COMBO, m_But7Combo);
	DDX_Control(pDX, IDC_BUT6COMBO, m_But6Combo);
	DDX_Control(pDX, IDC_BUT5COMBO, m_But5Combo);
	DDX_Control(pDX, IDC_BUT4COMBO, m_But4Combo);
	DDX_Control(pDX, IDC_BUT3COMBO, m_But3Combo);
	DDX_Control(pDX, IDC_BUT2COMBO, m_But2Combo);
	DDX_Control(pDX, IDC_BUT1COMBO, m_But1Combo);
	DDX_Control(pDX, IDC_A4MINCOMBO, m_A4MinCombo);
	DDX_Control(pDX, IDC_A4MAXCOMBO, m_A4MaxCombo);
	DDX_Control(pDX, IDC_A3MINCOMBO, m_A3MinCombo);
	DDX_Control(pDX, IDC_A3MAXCOMBO, m_A3MaxCombo);
	DDX_Control(pDX, IDC_A2MINCOMBO, m_A2MinCombo);
	DDX_Control(pDX, IDC_A1MINCOMBO, m_A1MinCombo);
	DDX_Control(pDX, IDC_A2MAXCOMBO, m_A2MaxCombo);
	DDX_Control(pDX, IDC_A1MAXCOMBO, m_A1MaxCombo);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	DDX_Control(pDX, IDC_DEBUGBEEP, m_DebugBeep);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPPJoyKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CPPJoyKeyDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DEBUGBEEP, OnDebugbeep)
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	ON_BN_CLICKED(IDC_LOADINI, OnLoadini)
	ON_BN_CLICKED(IDC_SAVEINI, OnSaveini)
	ON_CBN_SELCHANGE(IDC_A1MAXCOMBO, OnSelchangeA1maxcombo)
	ON_CBN_SELCHANGE(IDC_A1MINCOMBO, OnSelchangeA1mincombo)
	ON_CBN_SELCHANGE(IDC_A2MAXCOMBO, OnSelchangeA2maxcombo)
	ON_CBN_SELCHANGE(IDC_A2MINCOMBO, OnSelchangeA2mincombo)
	ON_CBN_SELCHANGE(IDC_A3MAXCOMBO, OnSelchangeA3maxcombo)
	ON_CBN_SELCHANGE(IDC_A3MINCOMBO, OnSelchangeA3mincombo)
	ON_CBN_SELCHANGE(IDC_A4MAXCOMBO, OnSelchangeA4maxcombo)
	ON_CBN_SELCHANGE(IDC_A4MINCOMBO, OnSelchangeA4mincombo)
	ON_CBN_SELCHANGE(IDC_BUT1COMBO, OnSelchangeBut1combo)
	ON_CBN_SELCHANGE(IDC_BUT2COMBO, OnSelchangeBut2combo)
	ON_CBN_SELCHANGE(IDC_BUT3COMBO, OnSelchangeBut3combo)
	ON_CBN_SELCHANGE(IDC_BUT4COMBO, OnSelchangeBut4combo)
	ON_CBN_SELCHANGE(IDC_BUT5COMBO, OnSelchangeBut5combo)
	ON_CBN_SELCHANGE(IDC_BUT6COMBO, OnSelchangeBut6combo)
	ON_CBN_SELCHANGE(IDC_BUT7COMBO, OnSelchangeBut7combo)
	ON_CBN_SELCHANGE(IDC_BUT8COMBO, OnSelchangeBut8combo)
	ON_BN_CLICKED(IDC_A2MINSCAN, OnA2minscan)
	ON_BN_CLICKED(IDC_A2MAXSCAN, OnA2maxscan)
	ON_BN_CLICKED(IDC_A1MINSCAN, OnA1minscan)
	ON_BN_CLICKED(IDC_A1MAXSCAN, OnA1maxscan)
	ON_BN_CLICKED(IDC_A3MINSCAN, OnA3minscan)
	ON_BN_CLICKED(IDC_A3MAXSCAN, OnA3maxscan)
	ON_BN_CLICKED(IDC_A4MINSCAN, OnA4minscan)
	ON_BN_CLICKED(IDC_A4MAXSCAN, OnA4maxscan)
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
	ON_CBN_SELCHANGE(IDC_BUT9COMBO, OnSelchangeBut9combo)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void LoadScancodeCombo (CComboBox& ComboBox)
{
 int	Count;
 int	Index;

 ComboBox.ResetContent();
 Count= 0;
 while (Scancodes[Count].Name)
 {
  Index= ComboBox.AddString(Scancodes[Count].Name);
  if (Index>=0)
   ComboBox.SetItemData (Index,Scancodes[Count].Scancode);
  Count++;
 }
}


/////////////////////////////////////////////////////////////////////////////
// CPPJoyKeyDlg message handlers

BOOL CPPJoyKeyDlg::OnInitDialog()
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

 // Setup default configuration
 LoadScancodeCombo (m_But16Combo);
 LoadScancodeCombo (m_But15Combo);
 LoadScancodeCombo (m_But14Combo);
 LoadScancodeCombo (m_But13Combo);
 LoadScancodeCombo (m_But12Combo);
 LoadScancodeCombo (m_But11Combo);
 LoadScancodeCombo (m_But10Combo);
 LoadScancodeCombo (m_But9Combo);
 LoadScancodeCombo (m_But8Combo);
 LoadScancodeCombo (m_But7Combo);
 LoadScancodeCombo (m_But6Combo);
 LoadScancodeCombo (m_But5Combo);
 LoadScancodeCombo (m_But4Combo);
 LoadScancodeCombo (m_But3Combo);
 LoadScancodeCombo (m_But2Combo);
 LoadScancodeCombo (m_But1Combo);
 LoadScancodeCombo (m_A4MinCombo);
 LoadScancodeCombo (m_A4MaxCombo);
 LoadScancodeCombo (m_A3MinCombo);
 LoadScancodeCombo (m_A3MaxCombo);
 LoadScancodeCombo (m_A2MinCombo);
 LoadScancodeCombo (m_A1MinCombo);
 LoadScancodeCombo (m_A2MaxCombo);
 LoadScancodeCombo (m_A1MaxCombo);

 m_DebugBeep.SetCheck(0);
 OnDebugbeep();

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();

 char			*CmdLine;

 CmdLine= AfxGetApp()->m_lpCmdLine;
 if (*CmdLine)
  LoadINI (CmdLine);

 return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPPJoyKeyDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPPJoyKeyDlg::OnPaint() 
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
HCURSOR CPPJoyKeyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


HRESULT ReadDirectInputData (void)
{
 HRESULT hr;
    
 if (!did)
  return S_OK;

 ZeroMemory(&DIKS,sizeof(DIKS));
 hr= did->GetDeviceState(sizeof(DIKS),&DIKS);
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

void UpdatePPJoy (void)
{
 JOYSTICK_STATE	js;
 int			Count;
 long			AxisValue;
 char			ButtonValue;
 DWORD			RetSize;
 DWORD			rc;

 // Make sure that the dialog window is already displayed. Else we simply return...
 if (!MainWindow)
  return;

 if (hVJoy==INVALID_HANDLE_VALUE)
  OpenPPJoyDevice();

 if (hVJoy==INVALID_HANDLE_VALUE)
  return;

 memset (&js,0,sizeof(js));
 js.Signature= JOYSTICK_STATE_V1;
 js.NumAnalog= NUM_ANALOG;
 js.NumDigital= NUM_DIGITAL;

 for (Count=0;Count<NUM_ANALOG;Count++)
 {
  AxisValue= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;
  if ((AxisMinIdx[Count]>=0)&&(AxisMinIdx[Count]<sizeof(DIKS))&&
      (AxisMaxIdx[Count]>=0)&&(AxisMaxIdx[Count]<sizeof(DIKS)))
  {
   if ((DIKS[AxisMinIdx[Count]]&0x80)&&(!(DIKS[AxisMaxIdx[Count]]&0x80)))
    AxisValue= PPJOY_AXIS_MIN;
   if ((!(DIKS[AxisMinIdx[Count]]&0x80))&&(DIKS[AxisMaxIdx[Count]]&0x80))
    AxisValue= PPJOY_AXIS_MAX;
  }
  js.Axis[Count]= AxisValue;
 }

 for (Count=0;Count<NUM_DIGITAL;Count++)
 {
  ButtonValue= 0;
  if ((ButtonIdx[Count]>=0)&&(ButtonIdx[Count]<sizeof(DIKS)))
  {
   if (DIKS[ButtonIdx[Count]]&0x80)
    ButtonValue= 1;
  }
  js.Digital[Count]= ButtonValue;
 }

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

  // Read keyboard input. If reading fails go back to start of the loop
  if (ReadDirectInputData()!=S_OK)
   continue;

  // Update PPJoy virtual stick
  UpdatePPJoy();

  if (DebugBeep)
   Beep(5000,30);
 }

 return 0;
}

int CPPJoyKeyDlg::DoModal() 
{
 CWinThread		*pThread;

 memset (AxisMinIdx,-1,sizeof(AxisMinIdx));
 memset (AxisMaxIdx,-1,sizeof(AxisMaxIdx));
 memset (ButtonIdx,-1,sizeof(ButtonIdx));

 *VJoyName= 0;
 hVJoy= INVALID_HANDLE_VALUE;

 MainWindow= NULL;	// Must sure the Window handle is NULL or valid...
 *INIFilename= 0;

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

/////////////////////////////////////////////////////////////////////////////
// Function to load and save INI files.                                    //
/////////////////////////////////////////////////////////////////////////////

void SetComboSel (CComboBox &ComboBox, int DesiredItemData)
{
 int	Count;

 Count= ComboBox.GetCount();
 while (Count--)
  if (ComboBox.GetItemData(Count)==(DWORD)DesiredItemData)
   break;
 ComboBox.SetCurSel(Count);
}

void CPPJoyKeyDlg::LoadINI (char *Filename)
{
 char	Buffer[1024];

 if (!GetPrivateProfileSectionNames (Buffer,sizeof(Buffer),Filename))
 {
  MessageBox ("Cannot read .ini file","Error",MB_OK);
  return;
 }

 m_DebugBeep.SetCheck(GetPrivateProfileInt(PPJOYKEY_APPNAME,"DebugBeep",0,Filename));
 OnDebugbeep();

 SetComboSel(m_JoyCombo,GetPrivateProfileInt(PPJOYKEY_APPNAME,"VirtualJoyNumber",1,Filename)-1);
 OnSelchangeJoycombo();

 SetComboSel (m_A1MinCombo,AxisMinIdx[0]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis1Min",0,Filename));
 SetComboSel (m_A2MinCombo,AxisMinIdx[1]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis2Min",0,Filename));
 SetComboSel (m_A3MinCombo,AxisMinIdx[2]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis3Min",0,Filename));
 SetComboSel (m_A4MinCombo,AxisMinIdx[3]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis4Min",0,Filename));

 SetComboSel (m_A1MaxCombo,AxisMaxIdx[0]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis1Max",0,Filename));
 SetComboSel (m_A2MaxCombo,AxisMaxIdx[1]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis2Max",0,Filename));
 SetComboSel (m_A3MaxCombo,AxisMaxIdx[2]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis3Max",0,Filename));
 SetComboSel (m_A4MaxCombo,AxisMaxIdx[3]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Axis4Max",0,Filename));

 SetComboSel (m_But1Combo,ButtonIdx[0]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button1",0,Filename));
 SetComboSel (m_But2Combo,ButtonIdx[1]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button2",0,Filename));
 SetComboSel (m_But3Combo,ButtonIdx[2]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button3",0,Filename));
 SetComboSel (m_But4Combo,ButtonIdx[3]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button4",0,Filename));
 SetComboSel (m_But5Combo,ButtonIdx[4]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button5",0,Filename));
 SetComboSel (m_But6Combo,ButtonIdx[5]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button6",0,Filename));
 SetComboSel (m_But7Combo,ButtonIdx[6]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button7",0,Filename));
 SetComboSel (m_But8Combo,ButtonIdx[7]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button8",0,Filename));

 SetComboSel (m_But9Combo,ButtonIdx[8]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button9",0,Filename));
 SetComboSel (m_But10Combo,ButtonIdx[9]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button10",0,Filename));
 SetComboSel (m_But11Combo,ButtonIdx[10]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button11",0,Filename));
 SetComboSel (m_But12Combo,ButtonIdx[11]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button12",0,Filename));
 SetComboSel (m_But13Combo,ButtonIdx[12]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button13",0,Filename));
 SetComboSel (m_But14Combo,ButtonIdx[13]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button14",0,Filename));
 SetComboSel (m_But15Combo,ButtonIdx[14]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button15",0,Filename));
 SetComboSel (m_But16Combo,ButtonIdx[15]= GetPrivateProfileInt(PPJOYKEY_APPNAME,"Button16",0,Filename));
}

// Wrapper to write an integer to a .INI file
BOOL WritePrivateProfileInt (LPCTSTR lpAppName, LPCTSTR lpKeyName, int Value, LPCTSTR lpFileName)
{
 char	Buffer[16];

 sprintf (Buffer,"%d",Value);
 return WritePrivateProfileString(lpAppName,lpKeyName,Buffer,lpFileName);	
}

void CPPJoyKeyDlg::SaveINI (char *Filename)
{
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"DebugBeep",m_DebugBeep.GetCheck(),Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"VirtualJoyNumber",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1,Filename);

 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis1Min",AxisMinIdx[0],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis2Min",AxisMinIdx[1],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis3Min",AxisMinIdx[2],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis4Min",AxisMinIdx[3],Filename);

 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis1Max",AxisMaxIdx[0],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis2Max",AxisMaxIdx[1],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis3Max",AxisMaxIdx[2],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Axis4Max",AxisMaxIdx[3],Filename);

 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button1",ButtonIdx[0],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button2",ButtonIdx[1],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button3",ButtonIdx[2],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button4",ButtonIdx[3],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button5",ButtonIdx[4],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button6",ButtonIdx[5],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button7",ButtonIdx[6],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button8",ButtonIdx[7],Filename);

 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button9",ButtonIdx[8],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button10",ButtonIdx[9],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button11",ButtonIdx[10],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button12",ButtonIdx[11],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button13",ButtonIdx[12],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button14",ButtonIdx[13],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button15",ButtonIdx[14],Filename);
 WritePrivateProfileInt(PPJOYKEY_APPNAME,"Button16",ButtonIdx[15],Filename);
}

/////////////////////////////////////////////////////////////////////////////
// Update functions for non-scancode controls                              //
/////////////////////////////////////////////////////////////////////////////

void CPPJoyKeyDlg::OnDebugbeep() 
{
 DebugBeep= m_DebugBeep.GetCheck();
}

// We change the joystick in a two step process. First we modify the global
// joystick name, then we close the IOCTL handle to force the worker function
// to re-open the IOCTL handle with the new joystick name
void CPPJoyKeyDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\" JOY_IOCTL_DEV_NAME,m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"PPJoy Keyboard to Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);
}

// Display file-open dialog. If the user picks an INI file load it.
void CPPJoyKeyDlg::OnLoadini() 
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
void CPPJoyKeyDlg::OnSaveini() 
{
 CFileDialog	fd(FALSE,".ini");

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 if (fd.DoModal()==IDOK)
  SaveINI (fd.m_ofn.lpstrFile);
}

/////////////////////////////////////////////////////////////////////////////
// Functions to handle updates for the scancode mapping lists.              //
/////////////////////////////////////////////////////////////////////////////

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

void CPPJoyKeyDlg::OnSelchangeA1maxcombo() 
{
 AxisMaxIdx[0]= GetComboSel(m_A1MaxCombo); 
}

void CPPJoyKeyDlg::OnSelchangeA1mincombo() 
{
 AxisMinIdx[0]= GetComboSel(m_A1MinCombo);
}

void CPPJoyKeyDlg::OnSelchangeA2maxcombo() 
{
 AxisMaxIdx[1]= GetComboSel(m_A2MaxCombo); 
}

void CPPJoyKeyDlg::OnSelchangeA2mincombo() 
{
 AxisMinIdx[1]= GetComboSel(m_A2MinCombo);
}

void CPPJoyKeyDlg::OnSelchangeA3maxcombo() 
{
 AxisMaxIdx[2]= GetComboSel(m_A3MaxCombo);  
}

void CPPJoyKeyDlg::OnSelchangeA3mincombo() 
{
 AxisMinIdx[2]= GetComboSel(m_A3MinCombo);
}

void CPPJoyKeyDlg::OnSelchangeA4maxcombo() 
{
 AxisMaxIdx[3]= GetComboSel(m_A4MaxCombo); 
}

void CPPJoyKeyDlg::OnSelchangeA4mincombo() 
{
 AxisMinIdx[3]= GetComboSel(m_A4MinCombo);
}

void CPPJoyKeyDlg::OnSelchangeBut1combo() 
{
 ButtonIdx[0]= GetComboSel(m_But1Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut2combo() 
{
 ButtonIdx[1]= GetComboSel(m_But2Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut3combo() 
{
 ButtonIdx[2]= GetComboSel(m_But3Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut4combo() 
{
 ButtonIdx[3]= GetComboSel(m_But4Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut5combo() 
{
 ButtonIdx[4]= GetComboSel(m_But5Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut6combo() 
{
 ButtonIdx[5]= GetComboSel(m_But6Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut7combo() 
{
 ButtonIdx[6]= GetComboSel(m_But7Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut8combo() 
{
 ButtonIdx[7]= GetComboSel(m_But8Combo);
}

void CPPJoyKeyDlg::OnA2minscan() 
{
 SetComboSel (m_A2MinCombo,AxisMinIdx[1]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA2maxscan() 
{
 SetComboSel (m_A2MaxCombo,AxisMaxIdx[1]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA1minscan() 
{
 SetComboSel (m_A1MinCombo,AxisMinIdx[0]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA1maxscan() 
{
 SetComboSel (m_A1MaxCombo,AxisMaxIdx[0]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA3minscan() 
{
 SetComboSel (m_A3MinCombo,AxisMinIdx[2]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA3maxscan() 
{
 SetComboSel (m_A3MaxCombo,AxisMaxIdx[2]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA4minscan() 
{
 SetComboSel (m_A4MinCombo,AxisMinIdx[3]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnA4maxscan() 
{
 SetComboSel (m_A4MaxCombo,AxisMaxIdx[3]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut1scan() 
{
 SetComboSel (m_But1Combo,ButtonIdx[0]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut2scan() 
{
 SetComboSel (m_But2Combo,ButtonIdx[1]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut3scan() 
{
 SetComboSel (m_But3Combo,ButtonIdx[2]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut4scan() 
{
 SetComboSel (m_But4Combo,ButtonIdx[3]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut5scan() 
{
 SetComboSel (m_But5Combo,ButtonIdx[4]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut6scan() 
{
 SetComboSel (m_But6Combo,ButtonIdx[5]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut7scan() 
{
 SetComboSel (m_But7Combo,ButtonIdx[6]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut8scan() 
{
 SetComboSel (m_But8Combo,ButtonIdx[7]= ScanForKeypress());
}

int CPPJoyKeyDlg::ScanForKeypress()
{
 int			Count;
 CScanDialog	ScanDialog;

 for (Count=0;Count<sizeof(DIKS);Count++)
  if (DIKS[Count]&0x80)
   return Count;

 return ScanDialog.ScanForInput();
}

LRESULT CPPJoyKeyDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
{
 m_PPJoyStatus.SetWindowText (PPJoyStatus);
 return 0;
}


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

int CScanDialog::ScanForInput()
{
 if (DoModal()==IDCANCEL)
  return -1;

 return Scancode;
}

BOOL CScanDialog::OnInitDialog()
{
 CDialog::OnInitDialog();

 if (!SetTimer(1,10,NULL))
 {
  MessageBox ("Error setting timer to poll for keyboard input");
  EndDialog(IDCANCEL);
 }

 return TRUE;
}

void CScanDialog::OnTimer(UINT nIDEvent) 
{
 int	Count;

 for (Count=0;Count<sizeof(DIKS);Count++)
 {
  if (DIKS[Count]&0x80)
  {
   Scancode= Count;
   KillTimer (1);
   EndDialog(IDOK);
  }
 }
}

BOOL CScanDialog::PreTranslateMessage(MSG* pMsg) 
{
 int	Count;

 if ((pMsg->message==WM_KEYDOWN)||(pMsg->message==WM_KEYUP)||
	 (pMsg->message==WM_SYSKEYDOWN)||(pMsg->message==WM_SYSKEYUP))
 {
  for (Count=0;Count<sizeof(DIKS);Count++)
  {
   if (DIKS[Count]&0x80)
   {
    Scancode= Count;
    KillTimer (1);
    EndDialog(IDOK);
   }
  }
  return 1;
 }	

 return CDialog::PreTranslateMessage(pMsg);
}

void CScanDialog::OnCancel() 
{
 KillTimer (1);
	
 CDialog::OnCancel();
}

void CPPJoyKeyDlg::OnBut9scan() 
{
 SetComboSel (m_But9Combo,ButtonIdx[8]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut10scan() 
{
 SetComboSel (m_But10Combo,ButtonIdx[9]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut11scan() 
{
 SetComboSel (m_But11Combo,ButtonIdx[10]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut12scan() 
{
 SetComboSel (m_But12Combo,ButtonIdx[11]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut13scan() 
{
 SetComboSel (m_But13Combo,ButtonIdx[12]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut14scan() 
{
 SetComboSel (m_But14Combo,ButtonIdx[13]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut15scan() 
{
 SetComboSel (m_But15Combo,ButtonIdx[14]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnBut16scan() 
{
 SetComboSel (m_But16Combo,ButtonIdx[15]= ScanForKeypress());
}

void CPPJoyKeyDlg::OnSelchangeBut9combo() 
{
 ButtonIdx[8]= GetComboSel(m_But9Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut10combo() 
{
 ButtonIdx[9]= GetComboSel(m_But10Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut11combo() 
{
 ButtonIdx[10]= GetComboSel(m_But11Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut12combo() 
{
 ButtonIdx[11]= GetComboSel(m_But12Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut13combo() 
{
 ButtonIdx[12]= GetComboSel(m_But13Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut14combo() 
{
 ButtonIdx[13]= GetComboSel(m_But14Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut15combo() 
{
 ButtonIdx[14]= GetComboSel(m_But15Combo);
}

void CPPJoyKeyDlg::OnSelchangeBut16combo() 
{
 ButtonIdx[15]= GetComboSel(m_But16Combo);
}

int CPPJoyKeyDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
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

 hr= di->CreateDevice(GUID_SysKeyboard,&did,NULL);
 if (FAILED(hr))
 {
  MessageBox ("Error creating DirectInputDevice interface for the keyboard");
  goto Exit;
 }

 hr= did->SetCooperativeLevel(AfxGetMainWnd()->m_hWnd,DISCL_BACKGROUND|DISCL_NONEXCLUSIVE);
 if (FAILED(hr))
 {
  MessageBox ("Cannot set Cooperation level...");
  goto Exit;
 }

 hr= did->SetDataFormat(&c_dfDIKeyboard); 
 if (FAILED(hr))
 { 
  MessageBox ("Cannot set keyboard data format...");
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
  MessageBox ("Cannot acquire keyboard...");
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

void CPPJoyKeyDlg::OnDestroy() 
{
 if (did)
  did->Release();
 did= NULL;

 if (di)
  di->Release();
 di= NULL;

 CDialog::OnDestroy();
}

int CPPJoyKeyDlg::PopulateJoyCombo()
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

void CPPJoyKeyDlg::OnCancel()
{
 if (MessageBox ("Are you sure you want to close PPJoyKey?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)!=IDYES)
  return;

 // We unacquire the keyboard device here because the window handle is destroyed before
 // DoModal returns. DirectInput requires the window handle passed to Acquire() to remain
 // valid while the device (keyboard) is acquired.

 // Might as well also clear the event notification while we are at it...
 if (did)
 {
  did->Unacquire();
  did->SetEventNotification (NULL);
 }

 CDialog::OnCancel();
}

