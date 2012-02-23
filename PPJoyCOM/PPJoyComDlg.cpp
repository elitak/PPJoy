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


// PPJoyComDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PPJoyCom.h"
#include "PPJoyComDlg.h"

#define	SUBMIT_ON_SYNC


/////////////////////////////////////////////////////////////////////////////
// Include files for PPJoy IOCTL calls                                     //
/////////////////////////////////////////////////////////////////////////////
#include <winioctl.h>
#include <ppjioctl.h>
#include <setupapi.h>
#include <devguid.h>

#include "Debug.h"
#include "afxwin.h"

#include "PPJoyAPI.h"
#include "Branding.h"

/////////////////////////////////////////////////////////////////////////////
// Define constants used in the application                                //
/////////////////////////////////////////////////////////////////////////////
#define	NUM_ANALOG	16					// Num analog values sent to PPJoy
#define	NUM_DIGITAL	16					// Num digital values sent to PPJoy
#define	PPJOYCOM_APPNAME	"PPJoyCom"	// Section name for INI files

#define	NUM_PROTOCOLS		5

#define	WM_PPJOYSTATUS		WM_USER+1
#define	WM_SERIALSTATUS		WM_USER+2
#define	WM_STREAMSTATUS		WM_USER+3

#define	LOG_CHARSPERLINE	16
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

struct SERIAL_PROTOCOL
{
 char	*Name;
 int (*Init) (void);
 int (*Process) (BYTE SerialInput);
};

/////////////////////////////////////////////////////////////////////////////
// Global variables used through-out the application                       //
/////////////////////////////////////////////////////////////////////////////

HANDLE				hTerminateEvent;	// Signal scan thread to exit
CWinThread			*ScanThread;		// Handle to scan loop thread

//int					TerminateFlag;		// If >0 input read thread terminates
int					DebugBeep;			// If >0 Beep on serial input

HANDLE				hVJoy= INVALID_HANDLE_VALUE;		// Handle for IOCTL to current joystick
char				VJoyName[64];		// Name of the current joystick device
JOYSTICK_STATE		JoyState;			// Joystick data to send to PPJoy;

char				SerialPortName[64];	// Serial port to read
HANDLE				hSerialPort= INVALID_HANDLE_VALUE;	// Serial port device to read

DWORD				BaudRate;			// Bits per seconds
DWORD				DataBits;			// Number of bits in each byte received
DWORD				Parity;				// Parity of received data
DWORD				StopBits;			// Stop bits after each byte

DWORD				RTSState;			// State for RTS pin on serial port
DWORD				DTRState;			// State for DTR pin on serial port
DWORD				DSRFlow;			// Will we apply flow control based on DSR
DWORD				CTSFlow;			// Will we apply flow control based on CTS

int					ActiveProtocol= -1;	// Index of protocol currently selected
HWND				MainWindow;			// Window to receive status updates
CPPJoyComDlg		*PPJoyComDlg;		// Reference to dialog object

char				INIFilename[1024];	// Last INI file used (load or save)
char				PPJoyStatus[128];	// PPJoy status message
char				SerialStatus[128];	// Serial (port) status message
char				StreamStatus[128];	// Stream status message

FILE				*LogFile;			// File to log serial input and configuration.
char				LogFilename[1024]= "PPJoyCOM.log";	// Filename of the log file.
int					LogCharsCount;

int InitFMS9600 (void);
int InitFMS19200 (void);
int InitZhenHua (void);
int ProcessFMS9600FMSLead (BYTE SerialInput);
int ProcessFMS9600Generic (BYTE SerialInput);
int ProcessFMS19200 (BYTE SerialInput);
int ProcessZhenHua (BYTE SerialInput);

int InitAeroChop (void);
int ProcessAeroChop (BYTE SerialInput);

int OpenSerialPort (void);
void SetComboSel (CComboBox &ComboBox, int DesiredItemData);
void WriteCharToLog (BYTE InputChar);
void WriteComSettingsToLog (void);
void WriteProtocolToLog (void);


SERIAL_PROTOCOL		SerialProtocol[NUM_PROTOCOLS]=
					{
					{"FMS PIC 9600 baud (Generic)",InitFMS9600,ProcessFMS9600Generic},
					{"FMS PIC 9600 baud (0xF0+ sync)",InitFMS9600,ProcessFMS9600FMSLead},
					{"FMS PIC 19200 baud (0xFF sync)",InitFMS19200,ProcessFMS19200},
					{"Zhen Hua 5 byte protocol",InitZhenHua,ProcessZhenHua},
					{"AeroChopper serial",InitAeroChop,ProcessAeroChop}
					};

BYTE	SwapBitOrder[256]=
	{0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	 0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	 0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	 0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	 0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	 0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	 0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	 0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	 0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	 0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	 0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	 0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	 0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	 0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	 0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	 0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF};


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
    m_ComponentName.SetWindowText ("PPJoyCOM Version " VER_PRODUCTVERSION_STR);
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
// CPPJoyComDlg dialog

CPPJoyComDlg::CPPJoyComDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPPJoyComDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPPJoyComDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPPJoyComDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPPJoyComDlg)
	DDX_Control(pDX, IDC_LOGFILE, m_LogButton);
	DDX_Control(pDX, IDC_RTSCOMBO, m_RTSCombo);
	DDX_Control(pDX, IDC_DTRCOMBO, m_DTRCombo);
	DDX_Control(pDX, IDC_DSRCOMBO, m_DSRCombo);
	DDX_Control(pDX, IDC_CTSCOMBO, m_CTSCombo);
	DDX_Control(pDX, IDC_BITSCOMBO, m_BitsCombo);
	DDX_Control(pDX, IDC_STOPCOMBO, m_StopCombo);
	DDX_Control(pDX, IDC_BAUDCOMBO, m_BaudCombo);
	DDX_Control(pDX, IDC_SERIALCOMBO, m_SerialCombo);
	DDX_Control(pDX, IDC_PROTOCOLCOMBO, m_ProtocolCombo);
	DDX_Control(pDX, IDC_PARITYCOMBO, m_ParityCombo);
	DDX_Control(pDX, IDC_JOYCOMBO, m_JoyCombo);
	DDX_Control(pDX, IDC_DEBUGBEEP, m_DebugBeep);
	DDX_Control(pDX, IDC_SERIALSTATUS, m_SerialStatus);
	DDX_Control(pDX, IDC_PPJOYSTATUS, m_PPJoyStatus);
	DDX_Control(pDX, IDC_STREAMSTATUS, m_StreamStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPPJoyComDlg, CDialog)
	//{{AFX_MSG_MAP(CPPJoyComDlg)
    ON_MESSAGE(WM_PPJOYSTATUS, OnPPJoyStatus)
    ON_MESSAGE(WM_SERIALSTATUS, OnSerialStatus)
    ON_MESSAGE(WM_STREAMSTATUS, OnStreamStatus)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DEBUGBEEP, OnDebugbeep)
	ON_CBN_SELCHANGE(IDC_PARITYCOMBO, OnSelchangeParitycombo)
	ON_CBN_SELCHANGE(IDC_JOYCOMBO, OnSelchangeJoycombo)
	ON_BN_CLICKED(IDC_LOADINI, OnLoadini)
	ON_BN_CLICKED(IDC_SAVEINI, OnSaveini)
	ON_CBN_SELCHANGE(IDC_BITSCOMBO, OnSelchangeBitscombo)
	ON_CBN_SELCHANGE(IDC_BAUDCOMBO, OnSelchangeBaudcombo)
	ON_CBN_SELCHANGE(IDC_PROTOCOLCOMBO, OnSelchangeProtocolcombo)
	ON_CBN_SELCHANGE(IDC_SERIALCOMBO, OnSelchangeSerialcombo)
	ON_CBN_SELCHANGE(IDC_STOPCOMBO, OnSelchangeStopcombo)
	ON_CBN_SELCHANGE(IDC_RTSCOMBO, OnSelchangeRtscombo)
	ON_CBN_SELCHANGE(IDC_DTRCOMBO, OnSelchangeDtrcombo)
	ON_CBN_SELCHANGE(IDC_DSRCOMBO, OnSelchangeDsrcombo)
	ON_CBN_SELCHANGE(IDC_CTSCOMBO, OnSelchangeCtscombo)
	ON_BN_CLICKED(IDC_LOGFILE, OnLogfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int	NextByte;
int	UpdateStreamStatus;

int InitFMS9600 (void)
{
 PPJoyComDlg->m_BaudCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_BitsCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_ParityCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_StopCombo.EnableWindow(FALSE);

 BaudRate= 9600;
 DataBits= 8;
 Parity= NOPARITY;
 StopBits= ONESTOPBIT;

 SetComboSel (PPJoyComDlg->m_BaudCombo,BaudRate);
 SetComboSel (PPJoyComDlg->m_BitsCombo,DataBits);
 SetComboSel (PPJoyComDlg->m_ParityCombo,Parity);
 SetComboSel (PPJoyComDlg->m_StopCombo,StopBits);

 PPJoyComDlg->m_RTSCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DTRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DSRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_CTSCombo.EnableWindow(FALSE);

 RTSState= RTS_CONTROL_ENABLE;
 DTRState= DTR_CONTROL_DISABLE;
 DSRFlow= FALSE;
 CTSFlow= FALSE;
 
 SetComboSel (PPJoyComDlg->m_RTSCombo,RTSState);
 SetComboSel (PPJoyComDlg->m_DTRCombo,DTRState);
 SetComboSel (PPJoyComDlg->m_DSRCombo,DSRFlow);
 SetComboSel (PPJoyComDlg->m_CTSCombo,CTSFlow);

 NextByte= 0;
 UpdateStreamStatus= 1;

 return OpenSerialPort();
}

int InitFMS19200 (void)
{
 PPJoyComDlg->m_BaudCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_BitsCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_ParityCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_StopCombo.EnableWindow(FALSE);

 BaudRate= 19200;
 DataBits= 8;
 Parity= NOPARITY;
 StopBits= ONESTOPBIT;

 SetComboSel (PPJoyComDlg->m_BaudCombo,BaudRate);
 SetComboSel (PPJoyComDlg->m_BitsCombo,DataBits);
 SetComboSel (PPJoyComDlg->m_ParityCombo,Parity);
 SetComboSel (PPJoyComDlg->m_StopCombo,StopBits);

 PPJoyComDlg->m_RTSCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DTRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DSRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_CTSCombo.EnableWindow(FALSE);

 RTSState= RTS_CONTROL_ENABLE;
 DTRState= DTR_CONTROL_DISABLE;
 DSRFlow= FALSE;
 CTSFlow= FALSE;
 
 SetComboSel (PPJoyComDlg->m_RTSCombo,RTSState);
 SetComboSel (PPJoyComDlg->m_DTRCombo,DTRState);
 SetComboSel (PPJoyComDlg->m_DSRCombo,DSRFlow);
 SetComboSel (PPJoyComDlg->m_CTSCombo,CTSFlow);

 NextByte= 0;
 UpdateStreamStatus= 1;

 return OpenSerialPort();
}

int InitZhenHua (void)
{
 PPJoyComDlg->m_BaudCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_BitsCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_ParityCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_StopCombo.EnableWindow(FALSE);

 BaudRate= 19200;
 DataBits= 8;
 Parity= NOPARITY;
 StopBits= ONESTOPBIT;

 SetComboSel (PPJoyComDlg->m_BaudCombo,BaudRate);
 SetComboSel (PPJoyComDlg->m_BitsCombo,DataBits);
 SetComboSel (PPJoyComDlg->m_ParityCombo,Parity);
 SetComboSel (PPJoyComDlg->m_StopCombo,StopBits);

 PPJoyComDlg->m_RTSCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DTRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DSRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_CTSCombo.EnableWindow(FALSE);

 RTSState= RTS_CONTROL_ENABLE;
 DTRState= DTR_CONTROL_DISABLE;
 DSRFlow= FALSE;
 CTSFlow= FALSE;
 
 SetComboSel (PPJoyComDlg->m_RTSCombo,RTSState);
 SetComboSel (PPJoyComDlg->m_DTRCombo,DTRState);
 SetComboSel (PPJoyComDlg->m_DSRCombo,DSRFlow);
 SetComboSel (PPJoyComDlg->m_CTSCombo,CTSFlow);

 NextByte= 0;
 UpdateStreamStatus= 1;

 return OpenSerialPort();
}

int InitAeroChop (void)
{
 PPJoyComDlg->m_BaudCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_BitsCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_ParityCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_StopCombo.EnableWindow(FALSE);

 BaudRate= 4800;
 DataBits= 8;
 Parity= NOPARITY;
 StopBits= ONESTOPBIT;

 SetComboSel (PPJoyComDlg->m_BaudCombo,BaudRate);
 SetComboSel (PPJoyComDlg->m_BitsCombo,DataBits);
 SetComboSel (PPJoyComDlg->m_ParityCombo,Parity);
 SetComboSel (PPJoyComDlg->m_StopCombo,StopBits);

 PPJoyComDlg->m_RTSCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DTRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_DSRCombo.EnableWindow(FALSE);
 PPJoyComDlg->m_CTSCombo.EnableWindow(FALSE);

 RTSState= RTS_CONTROL_ENABLE;
 DTRState= DTR_CONTROL_DISABLE;
 DSRFlow= FALSE;
 CTSFlow= FALSE;
 
 SetComboSel (PPJoyComDlg->m_RTSCombo,RTSState);
 SetComboSel (PPJoyComDlg->m_DTRCombo,DTRState);
 SetComboSel (PPJoyComDlg->m_DSRCombo,DSRFlow);
 SetComboSel (PPJoyComDlg->m_CTSCombo,CTSFlow);

 NextByte= -2;
 UpdateStreamStatus= 1;

 return OpenSerialPort();
}

// Function to process input characters from a FMS PIC Serial r/c interface
int ProcessFMS9600FMSLead (BYTE SerialInput)
{
 static int	NumPulses;

 int		Count;

 if (NextByte==0)
 {
  if ((SerialInput<241)||(SerialInput>248))
  {
   sprintf (StreamStatus,"Invalid sync byte %d...",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   return 0;	// No update joystick
  }
  NumPulses= SerialInput - 240;
  NextByte++;

  if (UpdateStreamStatus)
  {
   sprintf (StreamStatus,"Reading serial stream");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 0;
  }
  return 0;	// No update joystick
 }

 if (NextByte==1)
 {
  NextByte++;
  for (Count=0;Count<8;Count++)
  {
   JoyState.Digital[Count]= SerialInput&1;
   SerialInput >>= 1;
  }
  return 1;
 }

 if ((NextByte>=2)&&(NextByte<(2+NumPulses)))
 {
  if ((SerialInput<100)||(SerialInput>200))
  {
   sprintf (StreamStatus,"Invalid pulse byte %d...",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   NextByte= 0;
   return 0;
  }
  JoyState.Axis[NextByte-2]= (SerialInput-100)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/100+PPJOY_AXIS_MIN;
  NextByte++;

  if (NextByte>=(2+NumPulses))
  {
   NextByte= 0;
   return 1;
  }
  return 0;
 }

 NextByte= 0;
 return 0;
}

// Function to process input characters from the 9600 baud FMS PIC Serial r/c interface
int ProcessFMS9600Generic (BYTE SerialInput)
{
 if (NextByte==0)
 {
  // At point we are expecting a sync byte
//  if (SerialInput!=0xF0)
  if (SerialInput<0xF0)
  {
   // We did not get the sync byte we expected
   sprintf (StreamStatus,"Invalid sync byte %d...",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;

   return 0;	// No update joystick
  }

  // We got the sync we wanted so lets move on to the next byte 
  NextByte++;
  if (UpdateStreamStatus)
  {
   sprintf (StreamStatus,"Reading serial stream");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 0;
  }
  return 0;	// No update joystick
 }

 if (NextByte==1)
 {
  // This is the "buttons" byte - we will populate the bit fields into the first a digital inputs into PPJoy.
  NextByte++;
  for (int Count=0;Count<8;Count++)
  {
   JoyState.Digital[Count]= SerialInput&1;
   SerialInput >>= 1;
  }
  return 0;	// No update joystick
 }

 if ((NextByte>=(1+1))&&(NextByte<=(NUM_ANALOG+1)))
 {
  if (SerialInput>=0xF0)
  {
   NextByte= 1;
#ifdef SUBMIT_ON_SYNC
   return 1;
#else
   sprintf (StreamStatus,"Unexpected sync byte in data stream...");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   return 0;
#endif
  }

  JoyState.Axis[NextByte-2]= (SerialInput)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/0xEF+PPJOY_AXIS_MIN;
  NextByte++;

  if (NextByte>(NUM_ANALOG+1))
  {
   NextByte= 0;
   return 1;
  }
  return 0;
 }

 // Huh - we should not get here!!!
 NextByte= 0;
 return 0;
}

int ProcessFMS19200 (BYTE SerialInput)
{
 if (NextByte==0)
 {
  // At point we are expecting a sync byte
  if (SerialInput!=0xFF)
  {
   // We did not get the sync byte we expected
   sprintf (StreamStatus,"Invalid sync byte %d...",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;

   return 0;	// No update joystick
  }

  // We got the sync we wanted so lets move on to the next byte 
  NextByte++;
  if (UpdateStreamStatus)
  {
   sprintf (StreamStatus,"Reading serial stream");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 0;
  }
  return 0;	// No update joystick
 }

 if ((NextByte>=1)&&(NextByte<=NUM_ANALOG))
 {
  if (SerialInput==0xFF)
  {
   NextByte= 1;
#ifdef SUBMIT_ON_SYNC
   return 1;
#else
   sprintf (StreamStatus,"Unexpected sync byte in data stream...");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   return 0;
#endif
  }

  JoyState.Axis[NextByte-1]= (SerialInput)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/0xFE+PPJOY_AXIS_MIN;
  NextByte++;

  if (NextByte>NUM_ANALOG)
  {
   NextByte= 0;
   return 1;
  }
  return 0;
 }

 // Huh - we should not get here!!!
 NextByte= 0;
 return 0;
}

int ProcessZhenHua (BYTE SerialInput)
{
 // Swap bit order in byte.
 SerialInput= SwapBitOrder[SerialInput];

 if (NextByte==0)
 {
  // At this point we are expecting a sync byte
  if (SerialInput<0xF0)
  {
   // We did not get the sync byte we expected
   sprintf (StreamStatus,"Invalid sync byte %d...",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;

   return 0;	// No update joystick
  }

  // We got the sync we wanted so lets move on to the next byte 
  NextByte++;
  if (UpdateStreamStatus)
  {
   sprintf (StreamStatus,"Reading serial stream");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 0;
  }
  return 0;	// No update joystick
 }

 if ((NextByte>=1)&&(NextByte<=NUM_ANALOG))	//If we #undef SUBMIT_ON_SYNC this will have to change from NUM_ANALOG ==> 4
 {
  if (SerialInput>=0xF0)
  {
   NextByte= 1;
#ifdef SUBMIT_ON_SYNC
   return 1;
#else
   sprintf (StreamStatus,"Unexpected sync byte in data stream...");
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   return 0;
#endif
  }

  JoyState.Axis[NextByte-1]= (SerialInput)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/0xEF+PPJOY_AXIS_MIN;
  NextByte++;

  if (NextByte>NUM_ANALOG)					//If we #undef SUBMIT_ON_SYNC this will have to change from NUM_ANALOG ==> 4
  {
   NextByte= 0;
   return 1;
  }
  return 0;
 }

 // Huh - we should not get here!!!
 NextByte= 0;
 return 0;
}

int ProcessAeroChop (BYTE SerialInput)
{
 static int		CellNo;
 static BYTE	StreamBuffer[5*32*2];
 static int		InsertPtr;
 static int		InsertCnt;
 BYTE			CellHeader;

 if (NextByte==-2)
 {
  memset (StreamBuffer,0,sizeof(StreamBuffer));
  InsertPtr= 0;
  InsertCnt= 0;

  sprintf (StreamStatus,"Trying to synchronise stream");
  PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
  UpdateStreamStatus= 1;
  NextByte= -1;
 }

 if (NextByte==-1)		// Hard work ahead - we must detect the sync pattern
 {
  // Add new serial input to circular read buffer;
  StreamBuffer[InsertPtr]= SerialInput;
  InsertPtr= (InsertPtr+1)%sizeof(StreamBuffer);
  if (InsertCnt<sizeof(StreamBuffer))
   InsertCnt++;

  // We will only try to sync once we have captured a complete frame.
  // Only check for start of a new frame
  if ((SerialInput!=0x0F)||(InsertCnt<(5*32)))
   return 0;

  // Now we will do some more serious checking
  for (CellNo=0;CellNo<32;CellNo++)
  {
//   int Index;
//   Index= (InsertPtr+sizeof(StreamBuffer)-1-(5*32)+(CellNo*5))%sizeof(StreamBuffer);
//   CellHeader= StreamBuffer[Index];

   CellHeader= StreamBuffer[(InsertPtr+sizeof(StreamBuffer)-1-(5*32)+(CellNo*5))%sizeof(StreamBuffer)];

   if ((CellHeader&0x0F)!=0x0F)		// Test for xF
    return 0;

   if (((CellHeader&0x10)>>4)!=(CellNo&0x01))
    return 0;

   if ( (!(CellNo&03)) && ((CellHeader>>4)!=(CellNo>>1)) )
    return 0;  
  }

  // We got the sync we wanted so lets move on to the next byte 
  CellNo= 0;
  NextByte= 0;

  sprintf (StreamStatus,"Reading serial stream");
  PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
  UpdateStreamStatus= 0;
 }

 // Handle Cell header
 if (NextByte==0)
 {
  NextByte++;

  if ((SerialInput&0x0F)!=0x0F)
  {
   // CellHeader marks bits not set
   sprintf (StreamStatus,"Invalid cell header %02X... markers not set",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   NextByte= -2;
   return 0;	// No update joystick
  }

  if (((SerialInput&0x10)>>4)!=(CellNo&0x01))
  {
   // CellHeader alternating bit mismatch
   sprintf (StreamStatus,"Invalid cell header %02X... alt bit wrong",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   NextByte= -2;
   return 0;	// No update joystick
  }

  if (CellNo&03)
  {
   // Not a ref CellHeader: ignore its contents for now - we don't understand it!
   return 0;
  }

  if ((SerialInput>>4)!=(CellNo>>1))
  {
   // CellHeader ref counter wrong
   sprintf (StreamStatus,"Invalid cell header %02X... missing ref header",SerialInput);
   PostMessage (MainWindow,WM_STREAMSTATUS,0,(long)StreamStatus);
   UpdateStreamStatus= 1;
   NextByte= -2;
  }

  return 0;	// No update joystick
 }

 // Handle bytes 2-5 of cell = Channel information
 if ((NextByte>=1)&&(NextByte<=4))
 {
  JoyState.Axis[NextByte-1]= (SerialInput)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/0xFF+PPJOY_AXIS_MIN;
  NextByte++;

  if (NextByte>4)
  {
   NextByte= 0;
   CellNo= (CellNo+1)%32;
   return 1;
  }
  return 0;
 }

 // Huh - we should not get here!!!
 NextByte= -2;
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
// Serial port stuff....
//////////////////////////////////////////////////////////////////////////////

int OpenSerialPort (void)
{
 DCB			SerialDCB;
 COMMTIMEOUTS	SerialTimeouts;

 if (hSerialPort!=INVALID_HANDLE_VALUE)
  CloseHandle(hSerialPort);

 hSerialPort= CreateFile (SerialPortName,GENERIC_READ|GENERIC_WRITE,0,NULL,
					OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
//					OPEN_EXISTING,0,NULL);
 if (hSerialPort==INVALID_HANDLE_VALUE)
 {
  sprintf (SerialStatus,"Error %d opening port",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  return 0;
 }

 if (!GetCommState(hSerialPort,&SerialDCB))
 {
  sprintf (SerialStatus,"Error %d getting serial DCB",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  CloseHandle(hSerialPort);
  hSerialPort= INVALID_HANDLE_VALUE;
  return 0;
 }
 
 SerialDCB.BaudRate= BaudRate;
 SerialDCB.ByteSize= (BYTE) DataBits;
 SerialDCB.Parity= (BYTE) Parity;
 SerialDCB.StopBits= (BYTE) StopBits;

 SerialDCB.fBinary= TRUE;
 SerialDCB.fParity= TRUE; 

 SerialDCB.fOutxCtsFlow= CTSFlow;
 SerialDCB.fRtsControl= RTSState;
 SerialDCB.fOutxDsrFlow= DSRFlow;
 SerialDCB.fDtrControl= DTRState;
 SerialDCB.fDsrSensitivity= FALSE;

 SerialDCB.fOutX= FALSE;
 SerialDCB.fInX= FALSE;
 SerialDCB.fErrorChar= FALSE;
 SerialDCB.fNull= FALSE;
 SerialDCB.fAbortOnError= FALSE;

 if (!SetCommState(hSerialPort,&SerialDCB))
 {
  sprintf (SerialStatus,"Error %d setting serial DCB",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  CloseHandle(hSerialPort);
  hSerialPort= INVALID_HANDLE_VALUE;
  return 0;
 }

 SerialTimeouts.ReadIntervalTimeout= MAXDWORD;
 SerialTimeouts.ReadTotalTimeoutMultiplier= MAXDWORD;
 SerialTimeouts.ReadTotalTimeoutConstant= 5000;			// 5 seconds
 SerialTimeouts.WriteTotalTimeoutMultiplier= 1000;		// 1 second
 SerialTimeouts.WriteTotalTimeoutConstant= 1000;		// 1 second

 if (!SetCommTimeouts(hSerialPort,&SerialTimeouts))
 {
  sprintf (SerialStatus,"Error %d setting timeouts",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  CloseHandle(hSerialPort);
  hSerialPort= INVALID_HANDLE_VALUE;
  return 0;
 }
 
 if (!SetCommMask(hSerialPort,EV_RXCHAR))
 {
  sprintf (SerialStatus,"Error %d setting event mask",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  CloseHandle(hSerialPort);
  hSerialPort= INVALID_HANDLE_VALUE;
  return 0;
 }

 WriteComSettingsToLog();

 sprintf (SerialStatus,"Successfully opened port");
 PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
 return 1;
}

UINT ScanLoop (LPVOID pParam)
{
 char		UpdateJoystick;
 char		ReadSerialData;

 BYTE		Buffer[16];
 DWORD		NumRead;
 DWORD		rc;

 DWORD		EventMask;
 OVERLAPPED	Overlapped;
 
 HANDLE		hReadEvent;
 HANDLE		EventArray[2];

 hReadEvent= CreateEvent(NULL,TRUE,FALSE,NULL);
 if (!hReadEvent)
 {
  sprintf (SerialStatus,"Error %d calling WaitCommEvent()",GetLastError());
  PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
  goto Exit;
 }

 EventArray[0]= hTerminateEvent;
 EventArray[1]= hReadEvent;

 memset (&Overlapped,0,sizeof(Overlapped));
 Overlapped.hEvent= hReadEvent;
 EventMask= EV_RXCHAR;
 ReadSerialData= 0;

 while (1)
 {
  // Wait for a character to happen on the serial port
  if (!WaitCommEvent (hSerialPort,&EventMask,&Overlapped))
  {
   rc= GetLastError();
   if (rc==ERROR_IO_PENDING)
   {
    if (WaitForMultipleObjects (2,EventArray,FALSE,INFINITE)==WAIT_OBJECT_0)
     break;
    GetOverlappedResult(hSerialPort,&Overlapped,&NumRead,TRUE);
   }
   else
   {
    sprintf (SerialStatus,"Error %d calling WaitCommEvent()",GetLastError());
    PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
    ReadSerialData= 0;
    break;
   }
  }

  if (!ReadFile(hSerialPort,Buffer,sizeof(Buffer),&NumRead,&Overlapped))
  {
   rc= GetLastError();
   if (rc!=ERROR_IO_PENDING)
   {
    sprintf (SerialStatus,"Error %d reading serial port",GetLastError());
    PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
    ReadSerialData= 0;
    continue;
   }
   if (WaitForMultipleObjects (2,EventArray,FALSE,INFINITE)==WAIT_OBJECT_0)
    break;
   GetOverlappedResult(hSerialPort,&Overlapped,&NumRead,TRUE);
  }

  //Must update serial status here if we read data successfully
  if (!ReadSerialData)
  {
   sprintf (SerialStatus,"Reading serial stream",GetLastError());
   PostMessage (MainWindow,WM_SERIALSTATUS,0,(long)SerialStatus);
   ReadSerialData= 1;
  }

  // Check to see if the terminate flag is set. If yes get out of here
  if (WaitForSingleObject(hTerminateEvent,0)!=WAIT_TIMEOUT)
   break;

  if ((ActiveProtocol<0)||(ActiveProtocol>=NUM_PROTOCOLS))
   continue;

  for (BYTE Count=0;Count<NumRead;Count++)
  {
   UpdateJoystick= SerialProtocol[ActiveProtocol].Process(Buffer[Count]);
   WriteCharToLog (Buffer[Count]);
   // Update PPJoy virtual stick
   if (UpdateJoystick)
   {
    UpdatePPJoy();
    if (DebugBeep)
     Beep(5000,1);
   }
  }
 }

Exit:
 if (hReadEvent)
  CloseHandle(hReadEvent);

 // Now close the open serial port...
 PurgeComm (hSerialPort,PURGE_RXABORT|PURGE_RXCLEAR);
 CloseHandle(hSerialPort);
 hSerialPort= INVALID_HANDLE_VALUE;

// Beep(2500,20);
//
// while (WaitForSingleObject (hTerminateEvent,1000)==WAIT_TIMEOUT)
//  Beep(5000,10);
//
// Beep(2500,20);

 return 0;
}

int CPPJoyComDlg::StartScanLoop()
{
 ResetEvent (hTerminateEvent);

 if ((ActiveProtocol<0)||(ActiveProtocol>=NUM_PROTOCOLS))
  return -1;

 WriteProtocolToLog();

 if (!SerialProtocol[ActiveProtocol].Init())
  return -2;

 LogCharsCount= 0;

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

void CPPJoyComDlg::StopScanLoop()
{
 if (!ScanThread)
  return;

 SetEvent (hTerminateEvent);

 WaitForSingleObject (ScanThread->m_hThread,INFINITE);
 delete ScanThread;

 ScanThread= NULL;
}

//////////////////////////////////////////////////////////////////////////////
// User interface routines
//////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPPJoyComDlg message handlers

BOOL CPPJoyComDlg::OnInitDialog()
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

 if (!PopulateCOMCombo())
 {
  MessageBox ("No serial ports detected.\n\nThis application requires a serial port to operate","Fatal error: no COM ports",MB_OK|MB_ICONERROR); 
  EndDialog(IDCANCEL);
 }

 MainWindow= AfxGetMainWnd()->m_hWnd;
 PPJoyComDlg= this;

 LoadComboValues();

 // Setup default configuration. Do not open port, just display the
 // default values...
 m_SerialCombo.SetCurSel(0);
 m_SerialCombo.GetWindowText(SerialPortName,sizeof(SerialPortName));

 BaudRate= 9600;
 DataBits= 8;
 Parity= NOPARITY;
 StopBits= ONESTOPBIT;

 SetComboSel (m_BaudCombo,BaudRate);
 SetComboSel (m_BitsCombo,DataBits);
 SetComboSel (m_ParityCombo,Parity);
 SetComboSel (m_StopCombo,StopBits);

 RTSState= RTS_CONTROL_HANDSHAKE;
 DTRState= DTR_CONTROL_HANDSHAKE;
 DSRFlow= TRUE;
 CTSFlow= TRUE;
 
 SetComboSel (m_RTSCombo,RTSState);
 SetComboSel (m_DTRCombo,DTRState);
 SetComboSel (m_DSRCombo,DSRFlow);
 SetComboSel (m_CTSCombo,CTSFlow);

 m_ProtocolCombo.SetCurSel(0);

 m_DebugBeep.SetCheck(0);
 OnDebugbeep();

 m_JoyCombo.SetCurSel(0);
 OnSelchangeJoycombo();

 char			*CmdLine;

 CmdLine= AfxGetApp()->m_lpCmdLine;
 if (*CmdLine)
  LoadINI (CmdLine);

 // Call this to (re)start the scan thread
 OnSelchangeProtocolcombo();


 return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPPJoyComDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPPJoyComDlg::OnPaint() 
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
HCURSOR CPPJoyComDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CPPJoyComDlg::OnPPJoyStatus(WPARAM wParam, LPARAM lParam)
{
 m_PPJoyStatus.SetWindowText (PPJoyStatus);
 return 0;
}

LRESULT CPPJoyComDlg::OnSerialStatus(WPARAM wParam, LPARAM lParam)
{
 m_SerialStatus.SetWindowText (SerialStatus);
 return 0;
}

LRESULT CPPJoyComDlg::OnStreamStatus(WPARAM wParam, LPARAM lParam)
{
 m_StreamStatus.SetWindowText (StreamStatus);
 return 0;
}

void CPPJoyComDlg::OnDebugbeep() 
{
 DebugBeep= m_DebugBeep.GetCheck();
}

// Close the existing PPJoy joystick device, then open a new one. There is a small chance of
// a race condition - but hopefully not too serious. Scan thread might get a Windows error
// stating that the handle has already been closed...
void CPPJoyComDlg::OnSelchangeJoycombo() 
{
 HANDLE	hTemp;
 char	WindowTitle[128];

 sprintf (VJoyName,"\\\\.\\" JOY_IOCTL_DEV_NAME,m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);

 hTemp= hVJoy;
 hVJoy= INVALID_HANDLE_VALUE;

 if (hTemp)
  CloseHandle(hTemp);

 OpenPPJoyDevice();

 sprintf (WindowTitle,"PPJoy Serial stream to Virtual Joystick %d",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1);
 this->SetWindowText (WindowTitle);
}

int CPPJoyComDlg::DoModal() 
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
 if (hSerialPort!=INVALID_HANDLE_VALUE)
  CloseHandle (hSerialPort);
 if (LogFile)
  fclose (LogFile);

 return IDCANCEL;
}

void CPPJoyComDlg::OnLoadini() 
{
 CFileDialog	fd(TRUE);

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 fd.m_ofn.Flags|= OFN_FILEMUSTEXIST;

 if (fd.DoModal()==IDOK)
 {
  LoadINI (fd.m_ofn.lpstrFile);
  OnSelchangeProtocolcombo();
 }
}

void CPPJoyComDlg::OnSaveini() 
{
 CFileDialog	fd(FALSE,".ini");

 fd.m_ofn.lpstrFilter= "INI Files\0*.ini\0All files\0*\0\0";
 fd.m_ofn.lpstrFile= INIFilename;
 fd.m_ofn.nMaxFile= sizeof(INIFilename);

 if (fd.DoModal()==IDOK)
  SaveINI (fd.m_ofn.lpstrFile);
}

void CPPJoyComDlg::SaveINI(char *Filename)
{
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"DebugBeep",m_DebugBeep.GetCheck(),Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"VirtualJoyNumber",m_JoyCombo.GetItemData(m_JoyCombo.GetCurSel())+1,Filename);

 WritePrivateProfileString(PPJOYCOM_APPNAME,"SerialPortName",SerialPortName,Filename);	
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"StopBitsConstant",StopBits,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"DataBits",DataBits,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"BaudRate",BaudRate,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"ParityConstant",Parity,Filename);

 WritePrivateProfileInt(PPJOYCOM_APPNAME,"RTSState",RTSState,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"DTRState",DTRState,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"DSRFlow",DSRFlow,Filename);
 WritePrivateProfileInt(PPJOYCOM_APPNAME,"CTSFlow",CTSFlow,Filename);

 WritePrivateProfileInt(PPJOYCOM_APPNAME,"SerialStreamType",m_ProtocolCombo.GetCurSel(),Filename);
}

void CPPJoyComDlg::LoadINI(char *Filename)
{
 char			Buffer[1024];

 if (!GetPrivateProfileSectionNames (Buffer,sizeof(Buffer),Filename))
 {
  MessageBox ("Cannot read .ini file","Error",MB_OK);
  return;
 }

 m_DebugBeep.SetCheck(GetPrivateProfileInt(PPJOYCOM_APPNAME,"DebugBeep",0,Filename));
 OnDebugbeep();

 SetComboSel(m_JoyCombo,GetPrivateProfileInt(PPJOYCOM_APPNAME,"VirtualJoyNumber",1,Filename)-1);
 OnSelchangeJoycombo();

 m_ProtocolCombo.SetCurSel(GetPrivateProfileInt(PPJOYCOM_APPNAME,"SerialStreamType",0,Filename));

 GetPrivateProfileString(PPJOYCOM_APPNAME,"SerialPortName","COM1",SerialPortName,sizeof(SerialPortName),Filename);
 m_SerialCombo.SetCurSel(m_SerialCombo.FindStringExact(-1,SerialPortName));

 BaudRate= GetPrivateProfileInt(PPJOYCOM_APPNAME,"BaudRate",0,Filename);
 DataBits= GetPrivateProfileInt(PPJOYCOM_APPNAME,"DataBits",0,Filename);
 Parity= GetPrivateProfileInt(PPJOYCOM_APPNAME,"ParityConstant",0,Filename);
 StopBits= GetPrivateProfileInt(PPJOYCOM_APPNAME,"StopBitsConstant",0,Filename);
 
 SetComboSel (m_BaudCombo,BaudRate);
 SetComboSel (m_BitsCombo,DataBits);
 SetComboSel (m_ParityCombo,Parity);
 SetComboSel (m_StopCombo,StopBits);

 RTSState= GetPrivateProfileInt(PPJOYCOM_APPNAME,"RTSState",0,Filename);
 DTRState= GetPrivateProfileInt(PPJOYCOM_APPNAME,"DTRState",0,Filename);
 DSRFlow= GetPrivateProfileInt(PPJOYCOM_APPNAME,"DSRFlow",0,Filename);
 CTSFlow= GetPrivateProfileInt(PPJOYCOM_APPNAME,"CTSFlow",0,Filename);
 
 SetComboSel (m_RTSCombo,RTSState);
 SetComboSel (m_DTRCombo,DTRState);
 SetComboSel (m_DSRCombo,DSRFlow);
 SetComboSel (m_CTSCombo,CTSFlow);
}

void CPPJoyComDlg::OnSelchangeSerialcombo() 
{
 m_SerialCombo.GetWindowText(SerialPortName,sizeof(SerialPortName));

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeBitscombo() 
{
 DataBits= GetComboSel (m_BitsCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeBaudcombo() 
{
 BaudRate= GetComboSel (m_BaudCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeParitycombo() 
{
 Parity= GetComboSel (m_ParityCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeStopcombo() 
{
 StopBits= GetComboSel (m_StopCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeProtocolcombo() 
{
 StopScanLoop();

 InitJoyState();
 ActiveProtocol= m_ProtocolCombo.GetCurSel();
 m_StreamStatus.SetWindowText("Waiting for stream init...");

 StartScanLoop();
}

void CPPJoyComDlg::LoadComboValues()
{
 int	Count;

 m_BitsCombo.ResetContent();
 AddComboItem (m_BitsCombo,"7",7);
 AddComboItem (m_BitsCombo,"8",8);

 m_BaudCombo.ResetContent();
 AddComboItem (m_BaudCombo,"300",300);
 AddComboItem (m_BaudCombo,"1200",1200);
 AddComboItem (m_BaudCombo,"2400",2400);
 AddComboItem (m_BaudCombo,"4800",4800);
 AddComboItem (m_BaudCombo,"9600",9600);
 AddComboItem (m_BaudCombo,"14400",14400);
 AddComboItem (m_BaudCombo,"19200",19200);
 AddComboItem (m_BaudCombo,"38400",38400);
 AddComboItem (m_BaudCombo,"57600",57600);
 AddComboItem (m_BaudCombo,"115200",115200);

 m_ParityCombo.ResetContent();
 AddComboItem (m_ParityCombo,"None",NOPARITY);
 AddComboItem (m_ParityCombo,"Even",EVENPARITY);
 AddComboItem (m_ParityCombo,"Odd",ODDPARITY);
 AddComboItem (m_ParityCombo,"Mark",MARKPARITY);
 AddComboItem (m_ParityCombo,"Space",SPACEPARITY);

 m_StopCombo.ResetContent();
 AddComboItem (m_StopCombo,"1",ONESTOPBIT);
 AddComboItem (m_StopCombo,"1.5",ONE5STOPBITS);
 AddComboItem (m_StopCombo,"2",TWOSTOPBITS);

 m_RTSCombo.ResetContent();
 AddComboItem (m_RTSCombo,"Enable",RTS_CONTROL_ENABLE);
 AddComboItem (m_RTSCombo,"Disable",RTS_CONTROL_DISABLE);
 AddComboItem (m_RTSCombo,"Handshake",RTS_CONTROL_HANDSHAKE);
 AddComboItem (m_RTSCombo,"Toggle",RTS_CONTROL_TOGGLE);

 m_CTSCombo.ResetContent();
 AddComboItem (m_CTSCombo,"True",TRUE);
 AddComboItem (m_CTSCombo,"False",FALSE);

 m_DTRCombo.ResetContent();
 AddComboItem (m_DTRCombo,"Enable",DTR_CONTROL_ENABLE);
 AddComboItem (m_DTRCombo,"Disable",DTR_CONTROL_DISABLE);
 AddComboItem (m_DTRCombo,"Handshake",DTR_CONTROL_HANDSHAKE);

 m_DSRCombo.ResetContent();
 AddComboItem (m_DSRCombo,"True",TRUE);
 AddComboItem (m_DSRCombo,"False",FALSE);

 m_ProtocolCombo.ResetContent();
 for (Count=0;Count<NUM_PROTOCOLS;Count++)
  m_ProtocolCombo.AddString(SerialProtocol[Count].Name);
}

void CPPJoyComDlg::OnSelchangeRtscombo() 
{
 RTSState= GetComboSel (m_RTSCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeDtrcombo() 
{
 DTRState= GetComboSel (m_DTRCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeDsrcombo() 
{
 DSRFlow= GetComboSel (m_DSRCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnSelchangeCtscombo() 
{
 CTSFlow= GetComboSel (m_CTSCombo);

 OnSelchangeProtocolcombo();		// Restart scan loop on new port
}

void CPPJoyComDlg::OnLogfile() 
{
 char	Buffer[1024];

 if (LogFile)
 {
  fclose (LogFile);
  LogFile= NULL;
  m_LogButton.SetWindowText ("Record l&og");
 }
 else
 {
  CFileDialog	fd(FALSE,".log");

  fd.m_ofn.lpstrFilter= "Log files\0*.log\0All files\0*\0\0";
  fd.m_ofn.lpstrFile= LogFilename;
  fd.m_ofn.nMaxFile= sizeof(LogFilename);

  if (fd.DoModal()!=IDOK)
   return;

  LogFile= fopen (fd.m_ofn.lpstrFile,"w");
  if (!LogFile)
  {
   sprintf (Buffer,"Error opening logfile %s",fd.m_ofn.lpstrFile);
   return;
  }
  m_LogButton.SetWindowText ("Close l&og");
  LogCharsCount= 0;
  WriteProtocolToLog();
  WriteComSettingsToLog();
 }
}


void WriteCharToLog (BYTE InputChar)
{
 if (!LogFile)			// Do we have somewhere to write all this to?
  return;

 if (!(LogCharsCount%LOG_CHARSPERLINE))
  fprintf (LogFile,"\n%08X: %02X",LogCharsCount,InputChar);
 else
  fprintf (LogFile," %02X",InputChar);
 LogCharsCount++;
}

void WriteComSettingsToLog (void)
{
 if (LogFile)
 {
  fprintf (LogFile,"Opened %s with settings %d %d",SerialPortName,BaudRate,DataBits);
  switch (Parity)
  {
   case EVENPARITY:		fprintf (LogFile,"E"); break;
   case MARKPARITY:		fprintf (LogFile,"M"); break;
   case NOPARITY:		fprintf (LogFile,"N"); break;
   case ODDPARITY:		fprintf (LogFile,"O"); break;
   case SPACEPARITY:	fprintf (LogFile,"S"); break;
   default:				fprintf (LogFile,"?"); break;
  }
  switch (StopBits)
  {
   case ONESTOPBIT:		fprintf (LogFile,"1"); break;
   case ONE5STOPBITS:	fprintf (LogFile,"1.5"); break;
   case TWOSTOPBITS:	fprintf (LogFile,"2"); break;
   default:				fprintf (LogFile,"?"); break;
  }

  fprintf (LogFile,"\n-CTS flow control: %s\n-DSR flow contol: %s\n-RTS control: ",CTSFlow?"TRUE":"FALSE",DSRFlow?"TRUE":"FALSE");
  switch (RTSState)
  {
   case RTS_CONTROL_DISABLE:	fprintf (LogFile,"RTS_CONTROL_DISABLE"); break;
   case RTS_CONTROL_ENABLE:		fprintf (LogFile,"RTS_CONTROL_ENABLE"); break;
   case RTS_CONTROL_HANDSHAKE:	fprintf (LogFile,"RTS_CONTROL_HANDSHAKE"); break;
   case RTS_CONTROL_TOGGLE:		fprintf (LogFile,"RTS_CONTROL_TOGGLE"); break;
   default:						fprintf (LogFile,"?"); break;
  }

  fprintf (LogFile,"\n-DTR control: ");
  switch (DTRState)
  {
   case DTR_CONTROL_DISABLE:	fprintf (LogFile,"DTR_CONTROL_DISABLE"); break;
   case DTR_CONTROL_ENABLE:		fprintf (LogFile,"DTR_CONTROL_ENABLE"); break;
   case DTR_CONTROL_HANDSHAKE:	fprintf (LogFile,"DTR_CONTROL_HANDSHAKE"); break;
   default:						fprintf (LogFile,"?"); break;
  }
  fprintf (LogFile,"\n");
 }
}

void WriteProtocolToLog (void)
{
 if (LogFile)
 {
  fprintf (LogFile,"\n\nSelected protocol %s\n",SerialProtocol[ActiveProtocol].Name);
 }
}

int CPPJoyComDlg::PopulateJoyCombo()
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

ULONG GetCOMNumber (char *COMName)
{
 ULONG	Number;

 Number= 0;
 /* Make sure the name starts with LPT */
 if ((COMName[0]=='C')&&(COMName[1]=='O')&&(COMName[2]=='M'))
 {
  COMName+= 3;
  /* Convert all following digits into a number. */
  while ((*COMName>='0')&&(*COMName<='9'))
   Number= Number*10+*(COMName++)-'0';

  /* If string contains other non-digits we got an error. */
  if (*COMName)
   Number= 0;
 }
 return Number;
}

int CPPJoyComDlg::PopulateCOMCombo()
{
 HDEVINFO			DeviceInfoSet;
 SP_DEVINFO_DATA	DeviceInfoData;
 HKEY				DeviceKey;

 DWORD				DeviceIndex;
 int				rc;
 char				Buffer[256];
 DWORD				BufSize;
 DWORD				ValueType;

 ULONG				COMNumber;
 int				Index;

 m_SerialCombo.ResetContent();

 // Use this GUID to get all Port (COM and LPT) type devices. Means we don't
 // need the DDK to build this application
 DeviceInfoSet= SetupDiGetClassDevs ((LPGUID)&GUID_DEVCLASS_PORTS,NULL,NULL,0);
 
 if(DeviceInfoSet==INVALID_HANDLE_VALUE)
 {
  DebugPrintf (("SetupDiGetClassDevs failed: %d\n", GetLastError()))
  return 0;
 }

 DeviceIndex= 0;
 while (1)
 {
  /* Initialise buffer so that SetupDiGetDeviceInterfaceDetail will accept it. */
  DeviceInfoData.cbSize= sizeof(SP_DEVINFO_DATA);
  if (!SetupDiEnumDeviceInfo (DeviceInfoSet,DeviceIndex++,&DeviceInfoData))
  {
   rc= GetLastError();
   if (rc!=ERROR_NO_MORE_ITEMS)
	DebugPrintf (("SetupDiEnumDeviceInfo failed %d\n",rc))
   break;
  }

  DeviceKey= SetupDiOpenDevRegKey (DeviceInfoSet,&DeviceInfoData,DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ);
  if (DeviceKey==INVALID_HANDLE_VALUE)
  {
   DebugPrintf (("SetupDiOpenDevRegKey failed %d\n",GetLastError()))
   continue;
  }

  BufSize= sizeof(Buffer);
  rc= RegQueryValueEx (DeviceKey,"PortName",NULL,&ValueType,(LPBYTE)Buffer,&BufSize);
  RegCloseKey (DeviceKey);

  if (rc!=ERROR_SUCCESS)
  {
   DebugPrintf (("RegQueryValueEx failed %d\n",GetLastError()))
   continue;
  }

  if (ValueType!=REG_SZ)
  {
   DebugPrintf (("Unexpected type for PortName value (%d)\n",ValueType))
   continue;
  }

  DebugPrintf (("Found port: %s\n",Buffer))

  COMNumber= GetCOMNumber(Buffer);
  if (!COMNumber)
  {
   DebugPrintf (("Port ignored - not a valid COM name\n"))
   continue;
  }
  
  Index= m_SerialCombo.AddString(Buffer);
  m_SerialCombo.SetItemData (Index,COMNumber);
 }

 SetupDiDestroyDeviceInfoList (DeviceInfoSet);
 return m_SerialCombo.GetCount()>0;	/* Need to relook return codes at a later stage... */
}

void CPPJoyComDlg::OnCancel() 
{
 if (MessageBox ("Are you sure you want to close PPJoyCOM?\n\nThe virtual joystick will stop updating if the\napplication is closed.","Confirm exit",MB_YESNO|MB_ICONQUESTION)==IDYES)
  CDialog::OnCancel();
}
