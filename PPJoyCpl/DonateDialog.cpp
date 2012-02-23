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



#include "stdafx.h"

#include <windows.h>
#include <urlmon.h>
#include <mshtmhst.h>

#include <atlbase.h>

#include "DonateDialog.h"
#include "resource.h"

#include <branding.h>

#include <TCHAR.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int ShowDonateDialog (void)
{
 VARIANT	OutputParam;

 HINSTANCE hinstMSHTML = LoadLibrary(_T("MSHTML.DLL"));
 
 if (hinstMSHTML)
 {
  SHOWHTMLDIALOGFN*	pfnShowHTMLDialog;
  pfnShowHTMLDialog= (SHOWHTMLDIALOGFN*)GetProcAddress(hinstMSHTML,"ShowHTMLDialog");	// Must be SBCS string
  if (pfnShowHTMLDialog)
  {
   IMoniker *pURLMoniker;
//   BSTR bstrURL = SysAllocString(L"file:///C:/Source/PPJoy/Test/HTMLtest/Debug/webpage.htm");
//   BSTR bstrURL = SysAllocString(L"res://PPortJoy.cpl//donate-page");
   BSTR bstrURL = SysAllocString(L"res://" LCPL_APPLET_FILENAME L"//#" MAKELSTR(MAKESTR(IDR_DONATE_PAGE)));		// Apply branding here at a later stage
   CreateURLMoniker(NULL, bstrURL, &pURLMoniker);

   if (pURLMoniker)
   {
	// The prototype is broken in the header file we use. pchOptions options is actuall WCHAR* not char*
    (*pfnShowHTMLDialog)(NULL,pURLMoniker,NULL,(WCHAR*)L"dialogHeight:440px;dialogWidth:440px;center:yes;help:no;resizable:no;status:no",&OutputParam);
	
    pURLMoniker->Release();
   }
   SysFreeString(bstrURL);
  }
  FreeLibrary(hinstMSHTML);
 }
 return ((OutputParam.vt=VT_BOOL)&&(OutputParam.boolVal));
}

void DonateDialog (void)
{
 CRegKey	RegKey;
 DWORD		DonateDialogVersion;

 RegKey.Create(HKEY_CURRENT_USER,_T(CPL_APPLET_REGISTRYKEY));

 if (RegKey.QueryDWORDValue(_T(DONATE_DIALOG_VALUE),DonateDialogVersion)!=ERROR_SUCCESS)
  DonateDialogVersion= 0;

 if (DonateDialogVersion<DONATE_DIALOG_VER)
  if (ShowDonateDialog())
   RegKey.SetDWORDValue(_T(DONATE_DIALOG_VALUE),DONATE_DIALOG_VER);
}

