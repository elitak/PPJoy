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
#include <Wbemidl.h>

#include "Testsign.h"
#include "Debug.h"

// Function to check if the currently loaded OS supports loading test signed
// drivers. This is done by looking for "TESTSIGNING" or "/TESTSIGNING" in the 
// HKLM\SYSTEM\CurrentControl\Control\SystemStartOptions key.
// Return 1 if the text is found, 0 if not found or <0 for error.
int GetRegistryTestsigningFlag (void)
{
 int	rc;
 HKEY	RegKey;
 DWORD	ValueType;
 char	StartOptions[2048];					// Allow for a LONG string vale
 DWORD	BufSize= sizeof(StartOptions)-3;	// -3 to reserve extra space for leading and trailing space and terminating null
 char	*Search;

 // Open the target registry key
 rc= RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control",&RegKey);
 if (rc)
 {
  DebugPrintf (("RegOpenKey() error %d\n",rc));
  return -1;
 }

 // Make sure the options string starts with a space, then copy registry value
 // to next character. This will ensure that there is always at least one space
 // in front of each option.
 StartOptions[0]= ' ';
 rc= RegQueryValueExA (RegKey,"SystemStartOptions",NULL,&ValueType,(LPBYTE)(StartOptions+1),&BufSize);
 RegCloseKey (RegKey);						// Ignore the return value
 if (rc)
 {
  DebugPrintf (("RegQueryValueEx() error %d\n",rc));
  return -2;
 }

 // Make sure the value is of the right type - otherwise we are lost
 if (ValueType!=REG_SZ)
 {
  DebugPrintf (("StartupOptions value type 0x%04X, expected 0x%04X (REG_SZ)\n",ValueType,REG_SZ));
  return -3;
 }

 // Add a forced termination to the string in case the registry value was not
 // zero terminated. BufSize+1 because we started copying into the second
 // character of the buffer
 StartOptions[BufSize+1]= 0;				
 DebugPrintf (("Startup options: '%s'\n",StartOptions+1));	// Skip our leading space

 // Now we replace any forward slashes with a space - some versions of windows
 // add them (Vista) and some don't (Windows 7). This makes searching easier.
 // We also append a final space to ensure all options are delimited with a
 // trailing space, even at the end of the string.
 Search= StartOptions;
 while (*Search)
 {
  if (*Search=='/') *Search=' ';
  Search++;
 }
 *(Search++)= ' ';			// Append a space to the end of the string
 *Search= 0;				// And ensure it is still terminated

 // Now just search for the option name surrounded by spaces - easy
 DebugPrintf (("Processed Startup options: '%s'\n",StartOptions));
 Search= strstr(StartOptions," TESTSIGNING ");

 // Returns 1 if Search is not NULL (i.e. Option found)
 return Search!=NULL;
}

//=================== COM functions for BCD Store operations ==================

// Does not seem to be defined in the include files so we define it here. Values
// and names taken from the BCD documentation in the Windows SDK.
#define	BcdLibraryBoolean_AllowPrereleaseSignatures		0x16000049 
#define	BcdOSLoaderBoolean_AllowPrereleaseSignatures	0x26000027

// COM helper macros - used to hide some of the repetiveness of the implementation
// Macro to check if Object is allocated (non-null) before releasing it
#define	RELEASE_COM_OBJ(x)	{if ((x)) (x)->Release(); }
// Make a Unicode (L"xxx") string from "xxx" as parameter
#define	LSTR(x)			LSTR_HELP(x)
#define	LSTR_HELP(x)	L##x
// (Common) initialisation code for routines to set arguments for a WMI function call
#define	SET_WMI_ARG_INIT(argtype)							\
			HRESULT	hr;										\
			VARIANT	var;									\
			VariantInit (&var);								\
			var.vt= argtype;
// (Common) code to call WMI function that sets an argument for a method call
#define	SET_WMI_ARG_EXEC(argtypename)						\
			hr= PropertyClass->Put(ArgName,0,&var,0);		\
			VariantClear(&var);								\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf (("Error 0x%08X trying to set " argtypename " argument %ls\n",hr,ArgName)); \
			 return 0;										\
			}												\
			return 1;

// (Common) code to call WMI function which retrieves an out parameter
#define	GET_WMI_ARG_INIT(argtype,argtypename)				\
			HRESULT	hr;										\
			VARIANT	var;									\
			CIMTYPE	vt_type;								\
			hr= PropertyClass->Get(ArgName,0,&var,&vt_type,0);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf (("Error 0x%08X trying to get (" argtypename ") argument %ls\n",hr,ArgName));	\
			 return 0;										\
			}												\
			if (vt_type!=argtype)							\
			{												\
			 DebugPrintf (("Unexpected return type 0x%02X, expecting " #argtype "\n",vt_type));	\
			 VariantClear (&var);							\
			 return 0;										\
			}
// Common code to clean up after calling WMI function to get out parameter
#define	GET_WMI_ARG_DONE									\
			VariantClear (&var);							\
			return 1;

// (Common) code to execute a WMI method - initialise variables and parameter list
#define	EXEC_WMI_METHOD_INIT(objclasspath,methodname)		\
			HRESULT	hr;										\
			int					BoolFuncResult;				\
			IWbemClassObject	*pObjectClass= NULL;		\
			IWbemClassObject	*pInParamsClass= NULL;		\
			IWbemClassObject	*pInParamsObj= NULL;		\
			IWbemClassObject	*pOutParamsObj= NULL;		\
			BSTR				ObjectClassPath= SysAllocString(LSTR(objclasspath));	\
			BSTR				MethodName= SysAllocString(LSTR(methodname));	\
			VARIANT				var;						\
			/* Create a copy of the class object */			\
			VariantInit (&var);								\
			hr= pWMINamespace->GetObject(ObjectClassPath,WBEM_FLAG_RETURN_WBEM_COMPLETE,NULL,&pObjectClass,NULL);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf(("pWMINamespace->GetObject() returned 0x%08X trying to get object %ls\n",hr,ObjectClassPath));	\
			 goto Cleanup;									\
			}												\
			/* Get method arguments from the class object */\
			pObjectClass->GetMethod(MethodName,0,&pInParamsClass,NULL);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf(("pObjectClass->GetMethod() returned 0x%08X trying to get parameters for %ls\n",hr,MethodName));	\
			 goto Cleanup;									\
			}												\
			/* Create instance of input parameters */		\
			hr= pInParamsClass->SpawnInstance(0,&pInParamsObj);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf(("pInParamsClass->SpawnInstance() returned 0x%08X\n",hr));	\
			 goto Cleanup;									\
			}
// (Common) code to execute a WMI method - execute the actual method
#define	EXEC_WMI_METHOD_EXEC(objinstance)					\
			/* Get WMI path to this object instance */		\
			hr= objinstance->Get(L"__Relpath",0,&var,NULL,NULL);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf((#objinstance "->Get(__Relpath) returned 0x%08X\n",hr));	\
			 goto Cleanup;									\
			}												\
			DebugPrintf ((#objinstance " __Relpath: %ls\n",var.bstrVal));	\
			/* Execute method with instance path as classpath */	\
			hr= pWMINamespace->ExecMethod(var.bstrVal,MethodName,0,NULL,pInParamsObj,&pOutParamsObj,NULL);	\
			if (hr!=WBEM_S_NO_ERROR)						\
			{												\
			 DebugPrintf(("pWMINamespace->ExecMethod() returned 0x%08X for classpath %ls method %ls\n",hr,var.bstrVal,MethodName));	\
			 goto Cleanup;									\
			}												\
			/* Test to see if we have success */			\
			if (!GetWMIBooleanReturnValue(pOutParamsObj,&BoolFuncResult))	\
			 goto Cleanup;									\
			DebugPrintf (("%ls->%ls result is %s\n",var.bstrVal,MethodName,BoolFuncResult?"TRUE":"FALSE"));
// (Common) code to execute a WMI method - cleanup all variables
#define	EXEC_WMI_METHOD_DONE								\
			Cleanup:										\
			VariantClear (&var);							\
			SysFreeString (MethodName);						\
			SysFreeString (ObjectClassPath);				\
			RELEASE_COM_OBJ(pInParamsClass)					\
			RELEASE_COM_OBJ(pInParamsObj)					\
			RELEASE_COM_OBJ(pOutParamsObj)					\
			RELEASE_COM_OBJ(pObjectClass)




// Print to object definition of a WMI (class) object - used for debugging
void PrintObjectText (IWbemClassObject *Obj)
{
#ifdef _DEBUG		// Set by Visual Studio when we have a debug target
 HRESULT			hr;
 BSTR				Text= NULL;

 hr= Obj->GetObjectText(0,&Text);
 if (hr!=WBEM_S_NO_ERROR)
 {
  DebugPrintf (("Obj->GetObjectText() returned 0x%08X\n",hr));
  return;
 }
 DebugPrintf (("Object text: %ls\n",Text));
 SysFreeString (Text);
#endif
}

// WMI helper function to set a string argument to a function call
int SetWMIStringArg (IWbemClassObject *PropertyClass, WCHAR* ArgName, WCHAR* Str)
{
 SET_WMI_ARG_INIT(VT_BSTR)					// Declare and initialise variables
 var.bstrVal= SysAllocString(Str);			// Copy string value into parameter variable
 if (var.bstrVal==NULL)
  return 0;
 SET_WMI_ARG_EXEC("string")					// Set parameter value and return result
}

// WMI helper function to set a DWORD argument to a function call
int SetWMIDWORDArg (IWbemClassObject *PropertyClass, WCHAR* ArgName, DWORD ArgValue)
{
 SET_WMI_ARG_INIT(VT_I4)					// Declare and initialise variables
 var.lVal= ArgValue;						// Copy DWORD value into parameter variable
 SET_WMI_ARG_EXEC("DWORD")					// Set parameter value and return result
}

// WMI helper function to set a boolean argument to a function call
int SetWMIBooleanArg (IWbemClassObject *PropertyClass, WCHAR* ArgName, int ArgValue)
{
 SET_WMI_ARG_INIT(VT_BOOL)					// Declare and initialise variables
 var.lVal= ArgValue?0xFFFF:0;				// Copy boolean value into parameter variable
 SET_WMI_ARG_EXEC("boolean")				// Set parameter value and return result
}

// WMI helper function to retrieve a WMI object returned from method call
int GetWMIObjectArg (IWbemClassObject *PropertyClass, WCHAR* ArgName, IWbemClassObject **Result)
{
 GET_WMI_ARG_INIT(VT_UNKNOWN,"IUnknown interface")	// Get variant result

 // Attempt to find an IWebClassObject interface from the returned pointer
 hr= var.punkVal->QueryInterface(IID_IWbemClassObject,(void**)Result);
 if (hr!=S_OK)
 {
  // Could not find the interface
  DebugPrintf (("QueryInterface for WbemClassObject result failled: 0x%08X\n",hr));
  VariantClear (&var);
  return 0;
 }

 GET_WMI_ARG_DONE							// Clean-up after the function call
}

// WMI helper function to retrieve a boolean value returned from method call
int GetWMIBooleanArg (IWbemClassObject *PropertyClass, WCHAR* ArgName, int *Result)
{
 GET_WMI_ARG_INIT(VT_BOOL,"boolean")		// Get variant result
 if (Result) *Result= var.boolVal;			// Save result value
 GET_WMI_ARG_DONE							// Clean-up after the function call
}

// WMI helper function to retrieve the boolean return value of a method call
int GetWMIBooleanReturnValue (IWbemClassObject *PropertyClass, int *Result)
{
 // Just a little wrapper around the GetWMIBoolean function
 return GetWMIBooleanArg (PropertyClass,L"ReturnValue",Result);
}

// Function to return a services pointer to the WMI namespace. COM must already
// be initialised. Only cleanup required is to release the returned (non null)
// pointer and (later) uninitialise COM. Return NULL on failure.
IWbemServices* OpenWMINamespace (void)
{
 HRESULT			hr;
 IWbemLocator		*pWMILocator= NULL;
 IWbemServices		*pWMINamespace= NULL;
 BSTR				WMINamespacePath= SysAllocString(L"\\\\.\\root\\wmi");

 // Set up security context for COM access
 hr= CoInitializeSecurity (NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,RPC_C_IMP_LEVEL_IMPERSONATE, NULL,EOAC_SECURE_REFS,NULL);
 if (FAILED(hr))
 {
  printf("CoInitializeSecurity() returned 0x%08X\n",hr);
  goto Cleanup;
 }

 // Create COM object which will connect to WMI layer
 hr= CoCreateInstance (CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,IID_IWbemLocator,(LPVOID*)&pWMILocator);
 if (FAILED(hr))
 {
  printf ("CoCreateInstance() returned 0x%08X\n",hr);
  goto Cleanup;
 }

 // Connect to the WMI layer
 hr= pWMILocator->ConnectServer (WMINamespacePath,NULL,NULL,NULL,0,NULL,NULL,&pWMINamespace);
 if (hr!=WBEM_S_NO_ERROR)
 {
  printf("pWMILocator->ConnectServer() returned 0x%08X trying to connect to %ls\n",hr,WMINamespacePath);
  goto Cleanup;
 }

 // Set security context for WMI calls to use (pretty much from the Platform SDK)
 hr= CoSetProxyBlanket (pWMINamespace,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);
 if (hr!=WBEM_S_NO_ERROR)
 {
  printf("CoSetProxyBlanket() returned 0x%08X\n",hr);
  goto Cleanup;
 }

Cleanup:
 SysFreeString (WMINamespacePath);
 RELEASE_COM_OBJ(pWMILocator)

 return pWMINamespace;
}

// Function to open the default BCD store - we from here we will open the
// default OS and get/set configuration values. Caller must release the
// returned object; returns NULL on error.
IWbemClassObject* OpenDefaultBCDStore (IWbemServices *pWMINamespace)
{
 IWbemClassObject	*pBCDStoreObject= NULL;
 // Standard initialisation
 EXEC_WMI_METHOD_INIT("BcdStore","OpenStore")
 // Set File parameter as "" - specify to open default store
 if (!SetWMIStringArg (pInParamsObj,L"File",L"")) goto Cleanup;
 // Call the actual method - use pObjectClass from init macro to call static member
 EXEC_WMI_METHOD_EXEC(pObjectClass)
 // Get output parameters if call was successful
 if (BoolFuncResult) GetWMIObjectArg (pOutParamsObj,L"Store",&pBCDStoreObject);
 // Cleanup
 EXEC_WMI_METHOD_DONE
 return pBCDStoreObject;
}

// Function to open the OSLoader BCDObject for the currently running OS. Caller
// must release the returned object; returns NULL on error.
IWbemClassObject* OpenDefaultOSLoader (IWbemServices *pWMINamespace, IWbemClassObject *pBCDStoreObject)
{
 IWbemClassObject	*pOSLoaderObject= NULL;
 // Standard initialisation
 EXEC_WMI_METHOD_INIT("BcdStore","OpenObject")
 // Set Currently active OS instance as parameter
 if (!SetWMIStringArg (pInParamsObj,L"Id",L"{FA926493-6F1C-4193-A414-58F0B2456D1E}")) goto Cleanup;
 // Call the actual method
 EXEC_WMI_METHOD_EXEC(pBCDStoreObject)
 // Get output parameters if call was successful
 if (BoolFuncResult) GetWMIObjectArg (pOutParamsObj,L"Object",&pOSLoaderObject);
 // Cleanup
 EXEC_WMI_METHOD_DONE
 return pOSLoaderObject;
}

// Function to return the value of a Boolean Element contained in an BCD Object.
// Return value is 0 if the function encountered an error or 1 if the function
// call was successful. If return value is 1 then *Value parameter is valid and
// can be either: -1 (element not found), 0 (element value false) or 1 (element
// value true)
int GetBooleanElementValue (IWbemServices *pWMINamespace, IWbemClassObject *pOSLoaderObject, DWORD ElementID, int *Value)
{
 int				BoolElementValue;
 int				FuncResult= 0;
 IWbemClassObject	*pElementObject= NULL;
 // Standard initialisation
 EXEC_WMI_METHOD_INIT("BcdObject","GetElement")
 // Set ElementID for which we want the value
 if (!SetWMIDWORDArg (pInParamsObj,L"Type",ElementID)) goto Cleanup;
 // Call the actual method
 EXEC_WMI_METHOD_EXEC(pOSLoaderObject)
 // Test function result 
 if (!BoolFuncResult)
 {
  FuncResult= 1;		// No (WMI/COM) errors so function call successful
  *Value= -1;			// but element not found
  goto Cleanup;
 }
 // Function call succcesfull - extract the boolean value
 if (!GetWMIObjectArg(pOutParamsObj,L"Element",&pElementObject)) goto Cleanup;
 if (!GetWMIBooleanArg(pElementObject,L"Boolean",&BoolElementValue)) goto Cleanup;
 // OK, we success
 FuncResult= 1;					// Everything OK
 *Value= BoolElementValue!=0;	// And return the element value (convert from variant Boolean values to C boolean values)
 // Cleanup
 EXEC_WMI_METHOD_DONE
 RELEASE_COM_OBJ(pElementObject)
 return FuncResult;
}

// Function to set the value of a Boolean Element contained in an BCD Object.
// Return value is 0 if the function encountered an error or 1 if the function
// call was successful and the new element value set.
int SetBooleanElementValue (IWbemServices *pWMINamespace, IWbemClassObject *pOSLoaderObject, DWORD ElementID, int Value)
{
 int				FuncResult= 0;
 // Standard initialisation
 EXEC_WMI_METHOD_INIT("BcdObject","SetBooleanElement")
 // Set the ElementID (type) and the desired value as parameters for method call
 if (!SetWMIDWORDArg (pInParamsObj,L"Type",ElementID)) goto Cleanup;
 if (!SetWMIBooleanArg (pInParamsObj,L"Boolean",Value)) goto Cleanup;
 // Call the actual method
 EXEC_WMI_METHOD_EXEC(pOSLoaderObject)
 // Test the return value to set our function result
 FuncResult= BoolFuncResult!=0;
 // Cleanup
 EXEC_WMI_METHOD_DONE
 return FuncResult;
}

// Function to delete an Element contained in an BCD Object. Return value is
// 0 if the function encountered an error or 1 if the element was deleted.
int DeleteBCDElement (IWbemServices *pWMINamespace, IWbemClassObject *pOSLoaderObject, DWORD ElementID)
{
 int				FuncResult= 0;
 // Standard initialisation
 EXEC_WMI_METHOD_INIT("BcdObject","DeleteElement")
 // Set the ElementID (type) and the desired value as parameters for method call
 if (!SetWMIDWORDArg (pInParamsObj,L"Type",ElementID)) goto Cleanup;
 // Call the actual method
 EXEC_WMI_METHOD_EXEC(pOSLoaderObject)
 // Test the return value to set our function result
 FuncResult= BoolFuncResult!=0;
 // Cleanup
 EXEC_WMI_METHOD_DONE
 return FuncResult;
}

// Function to get the value of the BcdLibraryBoolean_AllowPrereleaseSignatures
// or BcdOSLoaderBoolean_AllowPrereleaseSignatures boolean elements from the BCD
// for the current running OS instance.
// Return value is -1: error, 0: value not set or false and 1: value set
int GetBCDTestsigningFlag (void)
{
 int				FuncResult= -1;
 int				Value;

 HRESULT			hr;
 IWbemServices		*pWMINamespace= NULL;
 IWbemClassObject	*pBCDStoreObject= NULL;
 IWbemClassObject	*pOSLoaderObject= NULL;
 // Must initialise COM subsystem
 hr= CoInitialize(0);
 if (FAILED(hr))
 {
  DebugPrintf(("CoInitialize() returned 0x%08X\n",hr));
  return FuncResult;
 }
 // Open WMI namespace and get a pointer to the WMI services
 pWMINamespace= OpenWMINamespace();
 if (!pWMINamespace) goto Cleanup;
 // Open the default WMI store
 pBCDStoreObject= OpenDefaultBCDStore(pWMINamespace);
 if (!pBCDStoreObject) goto Cleanup;
 PrintObjectText (pBCDStoreObject);
 // Once we have the BCD store opened, open current OSLoader object
 pOSLoaderObject= OpenDefaultOSLoader(pWMINamespace,pBCDStoreObject);
 if (!pOSLoaderObject) goto Cleanup;
 PrintObjectText (pOSLoaderObject);

 // First try to get the "Library" version of the test signing flag 
 if (!GetBooleanElementValue(pWMINamespace,pOSLoaderObject,BcdLibraryBoolean_AllowPrereleaseSignatures,&Value)) goto Cleanup;
 DebugPrintf (("BcdLibraryBoolean_AllowPrereleaseSignatures element value %d\n",Value));
 if (Value<0)	// No WMI/COM errors but element not found
 {
  // If the "Library" version is not found we will try to open the "OSLoader" version instead 
  if (!GetBooleanElementValue(pWMINamespace,pOSLoaderObject,BcdOSLoaderBoolean_AllowPrereleaseSignatures,&Value)) goto Cleanup;
  printf ("BcdOSLoaderBoolean_AllowPrereleaseSignatures element value %d\n",Value);
 }
 // Function returns 1 if either of the two testsign elements were set as true.
 FuncResult= Value>0;

Cleanup:
 RELEASE_COM_OBJ(pOSLoaderObject)
 RELEASE_COM_OBJ(pBCDStoreObject)
 RELEASE_COM_OBJ(pWMINamespace)
 // Uninitialise COM. If somebody wants to use COM again they must initialise it
 CoUninitialize();

 return FuncResult;
}

// Function to set the value of the BcdLibraryBoolean_AllowPrereleaseSignatures
// element to true or false (allow/disallow test signing) and then delete the
// BcdOSLoaderBoolean_AllowPrereleaseSignatures boolean element.
// Return value is 0: value not set (error) or 1: value successfully set
int SetBCDTestsigningFlag (int NewFlag)
{
 int				FuncResult= -1;
 
 HRESULT			hr;
 IWbemServices		*pWMINamespace= NULL;
 IWbemClassObject	*pBCDStoreObject= NULL;
 IWbemClassObject	*pOSLoaderObject= NULL;
 // Must initialise COM subsystem
 hr= CoInitialize(0);
 if (FAILED(hr))
 {
  DebugPrintf(("CoInitialize() returned 0x%08X\n",hr));
  return FuncResult;
 }
 // Open WMI namespace and get a pointer to the WMI services
 pWMINamespace= OpenWMINamespace();
 if (!pWMINamespace) goto Cleanup;
 // Open the default WMI store
 pBCDStoreObject= OpenDefaultBCDStore(pWMINamespace);
 if (!pBCDStoreObject) goto Cleanup;
 PrintObjectText (pBCDStoreObject);
 // Once we have the BCD store opened, open current OSLoader object
 pOSLoaderObject= OpenDefaultOSLoader(pWMINamespace,pBCDStoreObject);
 if (!pOSLoaderObject) goto Cleanup;
 PrintObjectText (pOSLoaderObject);

 // Attempt to set "Library" version of the Testsigning flag - BCDEDIT seems to
 // set this one in preference to the "OSLoader" version.
 if (!SetBooleanElementValue(pWMINamespace,pOSLoaderObject,BcdLibraryBoolean_AllowPrereleaseSignatures,NewFlag))
  goto Cleanup;
 // OK, if we get here the element was succesfully set
 FuncResult= 1;
 // Delete the "OSLoader" version of the flag in case it exists (may interfere
 // with the "Library" version?!
 DeleteBCDElement(pWMINamespace,pOSLoaderObject,BcdOSLoaderBoolean_AllowPrereleaseSignatures);

Cleanup:
 RELEASE_COM_OBJ(pOSLoaderObject)
 RELEASE_COM_OBJ(pBCDStoreObject)
 RELEASE_COM_OBJ(pWMINamespace)
 // Uninitialise COM. If somebody wants to use COM again they must initialise it
 CoUninitialize();

 return FuncResult;
}

// Function to reboot the operating system (lets new BCD option value take
// effect). Return value is 0 on error and 1 on success.
int RebootSystem (void)
{
 HANDLE				hToken; 
 TOKEN_PRIVILEGES	TokenPriv; 
 DWORD				LastError;
 
 // Get a token for this process. 
 if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
 {
  DebugPrintf(("OpenProcessToken() error %d\n",GetLastError()));
  return (0);
 }
 
 // Get the LUID for the shutdown privilege. 
 LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&TokenPriv.Privileges[0].Luid); 
 TokenPriv.PrivilegeCount= 1;  // one privilege to set
 TokenPriv.Privileges[0].Attributes= SE_PRIVILEGE_ENABLED; 
 
 // Enable the shutdown privilege for this process. 
 AdjustTokenPrivileges (hToken,FALSE,&TokenPriv,0,(PTOKEN_PRIVILEGES)NULL,0); 
 LastError= GetLastError();
 if (LastError!=ERROR_SUCCESS) // At least some priviledges not assigned.
 {
  DebugPrintf(("AdjustTokenPrivileges() error %d\n",GetLastError()));
  return (0);
 }

 // Shut down the system and force all applications to close. 
 if (!ExitWindowsEx(EWX_REBOOT|EWX_FORCE,SHTDN_REASON_MAJOR_SOFTWARE|SHTDN_REASON_MINOR_INSTALLATION|SHTDN_REASON_FLAG_PLANNED))
  return 0; 

 return 1;
}
