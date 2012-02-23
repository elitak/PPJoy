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


// PPJoyCpl.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PPJoyCpl.h"
#include "MainDialog.h"

#include <CPL.h>
#include <TCHAR.h>

#include "Debug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CPPJoyCplApp

BEGIN_MESSAGE_MAP(CPPJoyCplApp, CWinApp)
	//{{AFX_MSG_MAP(CPPJoyCplApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPPJoyCplApp construction

CPPJoyCplApp::CPPJoyCplApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPPJoyCplApp object

CPPJoyCplApp theApp;


/////////////////////////////////////////////////////////////////////////////
// External entry point for Control Panel applet

LONG APIENTRY CPlApplet(HWND hwndCPl, UINT uMsg, LONG lParam1, LONG lParam2)
{
 // Must be included here!!!
 AFX_MANAGE_STATE(AfxGetStaticModuleState());

 LPCPLINFO lpCPlInfo; 
 
 DebugPrintf ((_T("CPlApplet called - uMsg= %d\n"),uMsg));
 
 switch (uMsg)
 { 
  case CPL_INIT:		// first message, sent once 
		return TRUE; 
 
  case CPL_GETCOUNT:	// second message, sent once 
		return 1; 
		break; 

  case CPL_INQUIRE:		// third message, sent once per application 
		lpCPlInfo= (LPCPLINFO) lParam2; 
		lpCPlInfo->lData= 0;	// Context returned with CPL_DLKCLK
		lpCPlInfo->idIcon= IDI_JOYSTICK;
		lpCPlInfo->idName= IDS_CPL_NAME;
		lpCPlInfo->idInfo= IDS_CPL_DESC;
		break; 

  case CPL_DBLCLK:		// application icon double-clicked 
		{
		 CWnd			cwnd;

		 cwnd.Attach(hwndCPl);

		 CMainDialog	MainDialog(&cwnd);

		 MainDialog.DoModal();

		 cwnd.Detach();
		}
		break; 
 
  case CPL_STOP:		// sent once per application before CPL_EXIT 
		break; 
 
  case CPL_EXIT:		// sent once before FreeLibrary is called 
		break; 
 
  default: 
		break; 
 } 
 return 0; 
} 
