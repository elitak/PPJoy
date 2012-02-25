@echo off

REM Set to the name of the sub-directories for the output files
SET PRODUCTDIR=___Product___\Release
SET DEBUGPRODDIR=___Product___\Debug

SET PPJOYDIR=%~dp0

ECHO Building PPJoy...

REM ============ Build options for customised PPJoy setup =======================
rem rem SET INSTALLBAT=PPJoy.bat
rem rem SET PPJOYZIP=PPJoySetup.zip
REM =============================================================================

REM ============ Compiler config ==============================
SET DDKROOT=C:\WinDDK\7600.16385.0
SET VSROOT=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE
SET VSE2005ROOT=C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE
SET VSE2005CMD=VCExpress.exe
REM ===========================================================

echo.
echo Starting build process
echo ======================
echo.

PUSHD %PPJOYDIR%

RD /s /Q "%PPJOYDIR%\%PRODUCTDIR%"
RD /s /Q "%PPJOYDIR%\%DEBUGPRODDIR%"

MD "%PPJOYDIR%\%PRODUCTDIR%"
MD "%PPJOYDIR%\%PRODUCTDIR%\__Support__"
MD "%PPJOYDIR%\%DEBUGPRODDIR%"
MD "%PPJOYDIR%\%DEBUGPRODDIR%\__Support__"
RD /s /Q "%PPJOYDIR%\Scripts"
MD "%PPJOYDIR%\Scripts"


echo.
echo =====================================================
echo   Building checked (debug) version of PPJoy drivers
echo =====================================================
echo.

SETLOCAL
call "%DDKROOT%\bin\setenv.bat" %DDKROOT% chk wxp
CD /D "%PPJOYDIR%"
build /c
ENDLOCAL

echo.
echo ====================================================
echo   Building free (release) version of PPJoy drivers
echo ====================================================
echo.

SETLOCAL
call "%DDKROOT%\bin\setenv.bat" %DDKROOT% fre wxp
CD /D "%PPJOYDIR%"
build /c
ENDLOCAL

echo.
echo ============================================================
echo   Building checked (debug) 64 bit version of PPJoy drivers
echo ============================================================
echo.

SETLOCAL
call "%DDKROOT%\bin\setenv.bat" %DDKROOT% chk x64 WLH
CD /D "%PPJOYDIR%"
build /c
ENDLOCAL

echo.
echo ===========================================================
echo   Building free (release) 64 bit version of PPJoy drivers
echo ===========================================================
echo.

SETLOCAL
call "%DDKROOT%\bin\setenv.bat" %DDKROOT% fre x64 WLH
CD /D "%PPJOYDIR%"
build /c
ENDLOCAL

echo.
echo ==================================================
echo   Building setup helper plugin (legacy platform)
echo ==================================================
echo.

cd /D "%PPJOYDIR%\SetupHelper"
"%VSE2005ROOT%\%VSE2005CMD%" SetupHelper.sln /rebuild "Debug|Win32"
"%VSE2005ROOT%\%VSE2005CMD%" SetupHelper.sln /rebuild "Release|Win32"

REM output is only in Buildlog.html - make plan later

SETLOCAL

REM Set env variables for command line compiler - will use is some cases
call "%VSROOT%"..\..\..\VC\bin\vcvars32.bat
SET INCLUDE=%INCLUDE%;"%DDKROOT%\inc\api
REM for some strange reason the wxp directory is missing the dxguid.lib library so we tack the Win 2003 srv dir on to the end.
SET LIB=%LIB%;"%DDKROOT%\lib\wxp\i386;"%DDKROOT%\lib\wnet\i386


echo.
echo ====================================
echo   Creating additional script files
echo ====================================
echo.

cd /D "%PPJOYDIR%\CreateBAT"
"%VSROOT%\devenv" CreateBAT.sln /rebuild "Release|Win32" /UseEnv
cd /D "%PPJOYDIR%\Scripts"
"%PPJOYDIR%\CreateBAT\Release\CreateBAT"

echo.
echo =====================================
echo   Creating Device Driver INF Files
echo =====================================
echo.

cd /D "%PPJOYDIR%\CreateINF"
"%VSROOT%\devenv" CreateINF.sln /rebuild "Release|Win32" /UseEnv
cd /D "%PPJOYDIR%\%PRODUCTDIR%"
"%PPJOYDIR%\CreateINF\Release\CreateINF"
cd /D "%PPJOYDIR%\%DEBUGPRODDIR%
"%PPJOYDIR%\CreateINF\Release\CreateINF"

echo.
echo =====================================
echo   Building AddJoyDrivers helper dll
echo =====================================
echo.

cd /D "%PPJOYDIR%\AddJoyDrivers"
"%VSROOT%\devenv" AddJoyDrivers.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" AddJoyDrivers.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =====================================
echo   Building Helper64 x64 helper
echo =====================================
echo.

REM Don't use environment override for x64 - we set 32 it env vars
cd /D "%PPJOYDIR%\Helper64"
"%VSROOT%\devenv" Helper64.sln /rebuild "Debug|x64"
"%VSROOT%\devenv" Helper64.sln /rebuild "Release|x64"

echo.
echo =====================================
echo   Building UnInst64 x64 helper
echo =====================================
echo.

REM Don't use environment override for x64 - we set 32 it env vars
cd /D "%PPJOYDIR%\UnInst64"
"%VSROOT%\devenv" UnInst64.sln /rebuild "Debug|x64"
"%VSROOT%\devenv" UnInst64.sln /rebuild "Release|x64"

echo.
echo =====================================
echo    Building ViseHelper helper dll
echo =====================================
echo.

cd /D "%PPJOYDIR%\ViseHelper"
"%VSROOT%\devenv" ViseHelper.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" ViseHelper.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =====================================
echo       Building PPJoyAPI library
echo =====================================
echo.

cd /D "%PPJOYDIR%\PPJoyAPI"
"%VSROOT%\devenv" PPJoyAPI.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyAPI.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =========================================
echo   Building IOCTLSample test application 
echo =========================================
echo.

cd /D "%PPJOYDIR%\IOCTLSample"
copy /y "%PPJOYDIR%\Scripts\ppjioctl_devname.h"
"%VSROOT%\devenv" IOCTLSample.sln /rebuild "Release|Win32" /UseEnv



echo.
echo ============================================
echo   Building PPJoy Control Panel application
echo ============================================
echo.

cd /D "%PPJOYDIR%\PPJoyCpl"
"%VSROOT%\devenv" PPJoyCpl.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyCpl.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =================================
echo   Building PPJoyCOM application
echo =================================
echo.

cd /D "%PPJOYDIR%\PPJoyCOM"
"%VSROOT%\devenv" PPJoyCOM.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyCOM.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =================================
echo   Building PPJoyDLL application
echo =================================
echo.

cd /D "%PPJOYDIR%\PPJoyDLL"
"%VSROOT%\devenv" PPJoyDLL.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyDLL.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =================================
echo   Building PPJoyJoy application
echo =================================
echo.

cd /D "%PPJOYDIR%\PPJoyJoy"
"%VSROOT%\devenv" PPJoyJoy.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyJoy.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =================================
echo   Building PPJoyKey application
echo =================================
echo.

cd /D "%PPJOYDIR%\PPJoyKey"
"%VSROOT%\devenv" PPJoyKey.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyKey.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =================================
echo   Building PPJoyMouse application
echo =================================
echo.

cd /D "%PPJOYDIR%\PPJoyMouse"
"%VSROOT%\devenv" PPJoyMouse.sln /rebuild "Debug|Win32" /UseEnv
"%VSROOT%\devenv" PPJoyMouse.sln /rebuild "Release|Win32" /UseEnv

echo.
echo =====================================
echo   Building input .DLLs for PPJoyDLL
echo =====================================
echo.

cd /D "%PPJOYDIR%\RCCallbackDLLs"
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Debug Futaba_PCM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Debug Futaba_PPM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Debug JR_PCM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Debug JR_PPM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Release Futaba_PCM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Release Futaba_PPM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Release JR_PCM|Win32" /UseEnv
"%VSROOT%\devenv" RCCallbackDLLs.sln /rebuild "Release JR_PPM|Win32" /UseEnv



echo.
echo ===============================================
echo   Copying compiled files to product directory
echo ===============================================
echo.

cd /D "%PPJOYDIR%"
call Scripts\CopyProducts.bat %PRODUCTDIR% %DEBUGPRODDIR%
REM use for test certificate in an PFX exported file. DO NOT EXPORT real certificate to a PFX file
call Scripts\SignDriverFiles.bat %DDKROOT% /f TestSign\TestCertificate.pfx %PRODUCTDIR% %DEBUGPRODDIR%
REM
REM For real certificates use:
REM call Scripts\SignDriverFiles.bat %DDKROOT% /s <CertStoreName> %PRODUCTDIR% %DEBUGPRODDIR%
REM

copy /y IOCTLSample\Release\IOCTLSample.exe %PRODUCTDIR%\__Support__
copy /y IOCTLSample\Release\IOCTLSample.exe %DEBUGPRODDIR%\__Support__
copy /y AddJoyDrivers\Release\AddJoyDrivers.dll %PRODUCTDIR%\__Support__
copy /y AddJoyDrivers\Debug\AddJoyDrivers.dll %DEBUGPRODDIR%\__Support__
copy /y ViseHelper\Release\ViseHelper.dll %PRODUCTDIR%\__Support__
copy /y ViseHelper\Debug\ViseHelper.dll %DEBUGPRODDIR%\__Support__
copy /y SetupHelper\Release\SetupHelper.dll %PRODUCTDIR%\__Support__
copy /y SetupHelper\Debug\SetupHelper.dll %DEBUGPRODDIR%\__Support__
copy /y Helper64\x64\Release\Helper64.exe %PRODUCTDIR%\__Support__
copy /y Helper64\x64\Debug\Helper64.exe %DEBUGPRODDIR%\__Support__
copy /y UnInst64\x64\Release\UnInst64.exe %PRODUCTDIR%\__Support__
copy /y UnInst64\x64\Debug\UnInst64.exe %DEBUGPRODDIR%\__Support__
copy /y PPJoyAPI\Release\PPJoyAPI.lib %PRODUCTDIR%\__Support__
copy /y PPJoyAPI\Debug\PPJoyAPI.lib %DEBUGPRODDIR%\__Support__
copy /y PPJoyAPI\PPJoyAPI.h %PRODUCTDIR%\__Support__
copy /y Tools\AddDriversTest\AddDriversTest.c %PRODUCTDIR%\__Support__


copy /y PPJoyCOM\Release\PPJoyCOM.exe %PRODUCTDIR%
copy /y PPJoyCOM\Debug\PPJoyCOM.exe %DEBUGPRODDIR%
copy /y PPJoyDLL\Release\PPJoyDLL.exe %PRODUCTDIR%
copy /y PPJoyDLL\Debug\PPJoyDLL.exe %DEBUGPRODDIR%

copy /y PPJoyJoy\Release\PPJoyJoy.exe %PRODUCTDIR%
copy /y PPJoyJoy\Debug\PPJoyJoy.exe %DEBUGPRODDIR%
copy /y PPJoyKey\Release\PPJoyKey.exe %PRODUCTDIR%
copy /y PPJoyKey\Debug\PPJoyKey.exe %DEBUGPRODDIR%

copy /y PPJoyMouse\Release\PPJoyMouse.exe %PRODUCTDIR%
copy /y PPJoyMouse\Debug\PPJoyMouse.exe %DEBUGPRODDIR%

copy /y RCCallbackDLLs\Release\*.dll %PRODUCTDIR%
copy /y RCCallbackDLLs\Debug\*.dll %DEBUGPRODDIR%

zip -9 -X -j %PPJOYDIR%\Docs\Diagrams\Virtual\IOCTLSample.zip %PPJOYDIR%\IOCTLSample\*
zip -9 -X -j %PPJOYDIR%\Docs\Diagrams\Virtual\RCCallbackDLLs.zip %PPJOYDIR%\RCCallbackDLLs\*

xcopy /s /i docs %PRODUCTDIR%\docs

echo.
echo ======================================================
echo   Building PPJoyInstaller
echo ======================================================
echo.

"C:\Program Files (x86)\NSIS\makensis.exe" Installer\PPJoyInstaller.nsi

echo Build/Copy done.

ENDLOCAL

POPD


