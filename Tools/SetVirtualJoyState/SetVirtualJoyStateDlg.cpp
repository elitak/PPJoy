// SetVirtualJoyStateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetVirtualJoyState.h"
#include "SetVirtualJoyStateDlg.h"


/////////////////////////////////////////////////////////////////////////////
// Include files for DirectInput and PPJoy IOCTL calls                     //
/////////////////////////////////////////////////////////////////////////////
#include <winioctl.h>
#include <ppjioctl.h>

#include "JoyBus.h"

/////////////////////////////////////////////////////////////////////////////
// Define constants used in the application                                //
/////////////////////////////////////////////////////////////////////////////
#define	NUM_ANALOG	8					// Num analog values sent to PPJoy
#define	NUM_DIGITAL	16					// Num digital values sent to PPJoy
#define	PPJOYJOY_APPNAME	"PPJoyJoy"	// Section name for INI files

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

										// Fields for mix information
										// PPJoy related fields
HANDLE				hVJoy= INVALID_HANDLE_VALUE;
										// Handle for IOCTL to current joystick
char				VJoyName[64];		// Name of the current joystick device
JOYSTICK_STATE		JoyState;			// Joystick data to send to PPJoy;
char				PPJoyStatus[128];	// PPJoy status message

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// PPJoy virtual joystick functions                                        //
/////////////////////////////////////////////////////////////////////////////

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

LRESULT CSetVirtualJoyStateDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
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

/////////////////////////////////////////////////////////////////////////////
// End PPJoy virtual joystick functions                                        //
/////////////////////////////////////////////////////////////////////////////

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
	DECLARE_MESSAGE_MAP()
};

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
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetVirtualJoyStateDlg dialog

CSetVirtualJoyStateDlg::CSetVirtualJoyStateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetVirtualJoyStateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetVirtualJoyStateDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSetVirtualJoyStateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetVirtualJoyStateDlg)
	DDX_Control(pDX, IDC_DIGITALEDIT8, m_DigitalEdit8);
	DDX_Control(pDX, IDC_DIGITALEDIT7, m_DigitalEdit7);
	DDX_Control(pDX, IDC_DIGITALEDIT6, m_DigitalEdit6);
	DDX_Control(pDX, IDC_DIGITALEDIT5, m_DigitalEdit5);
	DDX_Control(pDX, IDC_DIGITALEDIT4, m_DigitalEdit4);
	DDX_Control(pDX, IDC_DIGITALEDIT2, m_DigitalEdit2);
	DDX_Control(pDX, IDC_DIGITALEDIT3, m_DigitalEdit3);
	DDX_Control(pDX, IDC_DIGITALEDIT1, m_DigitalEdit1);
	DDX_Control(pDX, IDC_ANALOGEDIT8, m_AnalogEdit8);
	DDX_Control(pDX, IDC_ANALOGEDIT7, m_AnalogEdit7);
	DDX_Control(pDX, IDC_ANALOGEDIT6, m_AnalogEdit6);
	DDX_Control(pDX, IDC_ANALOGEDIT5, m_AnalogEdit5);
	DDX_Control(pDX, IDC_ANALOGEDIT4, m_AnalogEdit4);
	DDX_Control(pDX, IDC_ANALOGEDIT3, m_AnalogEdit3);
	DDX_Control(pDX, IDC_ANALOGEDIT2, m_AnalogEdit2);
	DDX_Control(pDX, IDC_ANALOGEDIT1, m_AnalogEdit1);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetVirtualJoyStateDlg, CDialog)
	//{{AFX_MSG_MAP(CSetVirtualJoyStateDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetVirtualJoyStateDlg message handlers

BOOL CSetVirtualJoyStateDlg::OnInitDialog()
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
  MessageBox ("No PPJoy virtual joysticks detected\n\nYou must configure a virtual joystick in the PPJoy\nControl Panel before this application will work","Fatal error: no virtual joysticks",MB_OK|MB_ICONERROR); 
  EndDialog(IDCANCEL);
 }

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSetVirtualJoyStateDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSetVirtualJoyStateDlg::OnPaint() 
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
HCURSOR CSetVirtualJoyStateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSetVirtualJoyStateDlg::OnClose() 
{
 if (MessageBox ("Are you sure you want to close SetVirtualJoyState?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)==IDYES)
  CDialog::OnClose();
}






// PPJoyJoyDlg.cpp : implementation file

///////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Joystick reading and update routines
//////////////////////////////////////////////////////////////////////////////

#if 0

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
	ON_WM_CLOSE()
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
  MessageBox ("No PPJoy virtual joysticks detected\n\nYou must configure a virtual joystick in the PPJoy\nControl Panel before this application will work","Fatal error: no virtual joysticks",MB_OK|MB_ICONERROR); 
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



#endif

void CSetVirtualJoyStateDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\PPJoyIOCTL%d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"SetVirtualJoyState for Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);

 InitJoyState();
}

int CSetVirtualJoyStateDlg::PopulateJoyCombo()
{
 unsigned int	Count;
 char			JoystickName[64];
 int			Index;

 if (!OpenPPJoyBusDriver())
  return 0;

 m_JoyCombo.ResetContent();

 if (RescanJoysticks())
 {
  for (Count=0;Count<pEnumData->Count;Count++)
  {
   if (pEnumData->Joysticks[Count].JoyType==IF_IOCTL)
   {
    sprintf (JoystickName,"Virtual joystick %d",pEnumData->Joysticks[Count].UnitNumber+1);
    Index= m_JoyCombo.AddString(JoystickName);
    m_JoyCombo.SetItemData(Index,pEnumData->Joysticks[Count].UnitNumber);
   }
  }
 }

 ClosePPJoyBusDriver();

 return m_JoyCombo.GetCount()>0;
}


int CSetVirtualJoyStateDlg::DoModal() 
{
 CDialog::DoModal();

 if (hVJoy!=INVALID_HANDLE_VALUE)
  CloseHandle (hVJoy);

 return IDCANCEL;
}


void CSetVirtualJoyStateDlg::OnUpdate() 
{
 char	Buffer[128];

 m_AnalogEdit1.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[0]= atoi(Buffer);

 m_AnalogEdit2.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[1]= atoi(Buffer);

 m_AnalogEdit3.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[2]= atoi(Buffer);

 m_AnalogEdit4.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[3]= atoi(Buffer);

 m_AnalogEdit5.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[4]= atoi(Buffer);

 m_AnalogEdit6.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[5]= atoi(Buffer);

 m_AnalogEdit7.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[6]= atoi(Buffer);

 m_AnalogEdit8.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[7]= atoi(Buffer);

 m_AnalogEdit1.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[0]= atoi(Buffer);

 m_AnalogEdit2.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[1]= atoi(Buffer);

 m_AnalogEdit3.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Axis[2]= atoi(Buffer);


 m_DigitalEdit1.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[0]= atoi(Buffer);

 m_DigitalEdit2.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[1]= atoi(Buffer);

 m_DigitalEdit3.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[2]= atoi(Buffer);

 m_DigitalEdit4.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[3]= atoi(Buffer);

 m_DigitalEdit5.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[4]= atoi(Buffer);

 m_DigitalEdit6.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[5]= atoi(Buffer);

 m_DigitalEdit7.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[6]= atoi(Buffer);

 m_DigitalEdit8.GetWindowText(Buffer,sizeof(Buffer));
 JoyState.Digital[7]= atoi(Buffer);

 UpdatePPJoy();
}
