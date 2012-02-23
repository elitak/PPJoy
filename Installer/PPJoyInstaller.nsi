;
;   PPJoy Virtual Joystick for Microsoft Windows
;   Copyright (C) 2011 Deon van der Westhuysen
;
;   This program is free software: you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation, either version 3 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program.  If not, see <http://www.gnu.org/licenses/>.
;


; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "PPJoy Joystick Driver"
!define PRODUCT_VERSION "0.8.4.6"
!define PRODUCT_PUBLISHER "Deon van der Westhuysen"
!define PRODUCT_WEB_SITE "http://ppjoy.uni.cc"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Other global definitions
!define SOURCE_PRODUCT_DIR "..\___Product___\Release"
!define INSTALLER_DEST_DIR "..\___Product___"
!define SUPPORT_FILES_DIR  "${SOURCE_PRODUCT_DIR}\__Support__"
!define PPJOYCPL           "PPortJoy.cpl"

; Plugin directories for PPJoy helper plug-ins.
!addplugindir ${SUPPORT_FILES_DIR}

; Library includes...
!include "LogicLib.nsh"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\Branding\Joystick.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "PPJoyLicence.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOAUTOCLOSE
; Finish page
!define MUI_FINISHPAGE_RUN "$SYSDIR\control.exe"
;;!define MUI_FINISHPAGE_RUN_PARAMETERS  "$\"$INSTDIR\PPortJoy.cpl$\""
!define MUI_FINISHPAGE_RUN_PARAMETERS  "$\"$INSTDIR\${PPJOYCPL}$\""
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

;===================================================================================
;====================== Here the installer starts ==================================
;===================================================================================

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${INSTALLER_DEST_DIR}\PPJoySetup-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\PPJoy Joystick Driver"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

; Initialise the installer...
Function .onInit
  ; Check to see if we are on a supported OS version (Windows 2K or later)
  SetupHelper::nsis_IsSupportedOS
  ${If} $0 <> 1
    MessageBox MB_ICONSTOP|MB_OK "Cannot install ${PRODUCT_NAME} on this operating system. Windows 2000 or later required."
    Abort ; causes installer to quit.
  ${EndIf}

  SetupHelper::nsis_GetOSMajorVersion
  ${If} $0 == 5	
    ;; We are running on Windows 2000/XP. Test that the driver install policy allows unsigned drivers.
    ViseHelper::nsis_CheckSigningStatus
    ${If} $0 <> 1
      ;; This will change if we sign our drivers.
      MessageBox MB_ICONSTOP|MB_OK "Your system does not allow unsigned drivers to be installed.$\n$\nPlease change the driver signing setting in Control Panel, then restart the ${PRODUCT_NAME} installation."
      Abort ; causes installer to quit.
      ;; OK, actually we need to offer to try and turn off driver signing. [Are we sure??] Do this here.
      ;; --- TODO HERE ---
    ${EndIf}
  ${EndIf}

  ; Now, lets check if we are on a 64 bit system which needs signed drivers
  SetupHelper::nsis_Need64bitSignedDrivers
  ${If} $0 <> 0
    ;; This will fall away if we sign our drivers with a proper code signing certificate.
    ViseHelper::nsis_GetRegistryTestsigningFlag
    ${If} $0 == 1
      ;; OK, registry says the running OS instance supports unsigned drivers
      ViseHelper::nsis_GetBCDTestsigningFlag
      ${If} $0 == 1
        goto SystemAllowsTestSigned
      ${EndIf}
    ${EndIf}

    MessageBox MB_ICONSTOP|MB_OKCANCEL "Your 64 bit system does not allow the installation test signed drivers.$\n$\nClick OK to allow the ${PRODUCT_NAME} installation to make the required changes or CANCEL to change them yourself" IDCANCEL AbortTestSign
    ViseHelper::nsis_SetBCDTestsigningFlag 1
    MessageBox MB_ICONSTOP|MB_OKCANCEL "BCD Boot configuration data has been updated and will take effect with the next reboot.$\n$\nClick OK to reboot now or CANCEL if you plan to reboot later.$\n$\nPlease run installer again after the reboot." IDCANCEL AbortTestSign
    ViseHelper::nsis_RebootSystem
   AbortTestSign:
    Abort ; causes installer to quit.
   SystemAllowsTestSigned:
  ${EndIf}
FunctionEnd

;--- Section to initialise constant options
Section "-Init"
  SetOverwrite ifnewer
  SetShellVarContext all
SectionEnd

;--- First section: copy core PPJoy files and register the drivers, control panel
Section "PPJoyCore" SEC01
  ; Initialise section
  SetOutPath "$INSTDIR"
  
  ; Copy 64bit install helper to the plug-ins directory
  InitPluginsDir
  File "/oname=$PLUGINSDIR\Helper64.exe" "${SUPPORT_FILES_DIR}\Helper64.exe"

  ; OK, before we start copying new files, delete the ancient PPJoy version 0.50 bus driver
  ; if running. Then also delete DirectX joystick configuration associated with old PPJoy
  ViseHelper::nsis_DeletePPJoyDevices50
  ViseHelper::nsis_DeleteJoystickType "VID_DEAD&PID_BEEF"
  ;; Nevermind the return code for above two functions
  
  ; Copy PPJoy driver files
  File "${SOURCE_PRODUCT_DIR}\*.inf"
  ;; (Specifically exclude 64 bit driver files for now (/x "*64.sys")
  ;;  File /x "*64.sys" "${SOURCE_PRODUCT_DIR}\*.sys"
  File "${SOURCE_PRODUCT_DIR}\*.sys"
  File "${SOURCE_PRODUCT_DIR}\*.cat"
  ; Control Panel file
  File "${SOURCE_PRODUCT_DIR}\${PPJOYCPL}"

  ; Create shortcut for the PPJoy control panel
  CreateDirectory "$SMPROGRAMS\PPJoy Joystick Driver"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Configure Joysticks.lnk" "$SYSDIR\control.exe" "$\"$INSTDIR\${PPJOYCPL}$\"" "$INSTDIR\${PPJOYCPL}"

  ; Register control panel applet
  SetupHelper::nsis_GetOSName
  ${If} $0 == "Win2K"
    ; Register DLL for control panel - Win2K specific
    WriteINIStr "control.ini" "MMCPL" "${PPJOYCPL}" "$INSTDIR\${PPJOYCPL}"
  ${Else}
    ; Register DLL for control panel - XP and later.
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Cpls" "${PPJOYCPL}" "$INSTDIR\${PPJOYCPL}"

    ${If} $0 == "WinXP"
      ; Set control panel category - XP
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Extended Properties\{305CA226-D286-468e-B848-2B2E8E697B74} 2" "$INSTDIR\${PPJOYCPL}" 2
    ${Else}
      ; Set control panel category - Vista and later
      WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Extended Properties\System.ControlPanel.Category" "$INSTDIR\${PPJOYCPL}" 2
    ${EndIf}
  ${EndIf}
  
  ; Now install joystick bus driver
  AddJoyDrivers::nsis_Install "$INSTDIR"
  ${If} $0 == 0
    MessageBox MB_ICONEXCLAMATION|MB_OK "Error installing the new device drivers. PPJoy will not be operational. You will need to manually install the device drivers."
  ${EndIf}
  ${If} $0 == 1
    MessageBox MB_ICONEXCLAMATION|MB_OK "PPJoy driver install requires a reboot."
  ${EndIf}
  
  ;; Possibly add registery entry that controls the donate dialog? HKCU\Control Panel\PPJoy\DonateDialog = (DWORD) 2
SectionEnd

;--- Second section: copy the PPJoy documentation
Section "Documentation" SEC02
  ; Initialise section
  SetOutPath "$INSTDIR\Docs"

  ; Copy all the document files we can (recursively) find to the documents subdirectory
  File /r "${SOURCE_PRODUCT_DIR}\Docs\*"
  
  ; And create a start menu shortcut to the documenation index
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\PPJoy documentation.lnk" "$INSTDIR\Docs\PPJoyMain.htm"
SectionEnd

;--- Third section: copy the additional PPJoy virtual joystick input drivers
Section "MiniDrivers" SEC03
  ; Initialise section
  SetOutPath "$INSTDIR"

  ; Copy all the input mini-driver files we can find
  File "${SOURCE_PRODUCT_DIR}\PPJoy*.exe"
  File "${SOURCE_PRODUCT_DIR}\JR_*.dll"
  File "${SOURCE_PRODUCT_DIR}\Futaba_*.dll"
  
  ; And create shortcuts for them
  CreateDirectory "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Radio Control TX.lnk" "$INSTDIR\PPJoyCOM.exe"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\User DLL joystick.lnk" "$INSTDIR\PPJoyDLL.exe"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Keyboard joystick.lnk" "$INSTDIR\PPJoyKey.exe"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Joystick remixer.lnk" "$INSTDIR\PPJoyJoy.exe"
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Mouse joystick.lnk" "$INSTDIR\PPJoyMouse.exe"
SectionEnd

; Section to create uninstall icon
Section -AdditionalIcons
  CreateShortCut "$SMPROGRAMS\PPJoy Joystick Driver\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

; Section to add the uninstaller executable and uninstall registry entries.
Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PPJOYCPL}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

;===================================================================================
;====================== Here the uninstaller starts ================================
;===================================================================================

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ; Initialise section
  SetShellVarContext all

  ; Copy 64bit uninstall helper to the plug-ins directory
  InitPluginsDir
  File "/oname=$PLUGINSDIR\UnInst64.exe" "${SUPPORT_FILES_DIR}\UnInst64.exe"

  ; Remove the PPJoy bus driver.
  SetupHelper::nsis_UninstallDrv "$INSTDIR\${PPJOYCPL}"

  ; Remove the control panel registration (delete all possible entries, for every OS type)
  DeleteINIStr "control.ini" "MMCPL" "${PPJOYCPL}"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Cpls" "${PPJOYCPL}"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Extended Properties\{305CA226-D286-468e-B848-2B2E8E697B74} 2" "$INSTDIR\${PPJOYCPL}"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Control Panel\Extended Properties\System.ControlPanel.Category" "$INSTDIR\${PPJOYCPL}"

  ; Remove uninstall shortcut
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Uninstall.lnk"

  ; Remove the shortcuts to the program items
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Configure Joysticks.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\PPJoy documentation.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Radio Control TX.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\User DLL joystick.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Keyboard joystick.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Joystick remixer.lnk"
  Delete "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks\Mouse joystick.lnk"

  ; Remove start menu directories
  RMDir "$SMPROGRAMS\PPJoy Joystick Driver\Virtual Joysticks"
  RMDir "$SMPROGRAMS\PPJoy Joystick Driver"

  ; Delete all the files from the installation directory
  Delete "$INSTDIR\uninst.exe"
  RMDir /r /REBOOTOK "$INSTDIR\Docs"
  Delete "$INSTDIR\PPJoy*.exe"
  Delete "$INSTDIR\JR_*.dll"
  Delete "$INSTDIR\Futaba_*.dll"
  Delete "$INSTDIR\Example.file"
  Delete "$INSTDIR\W98Ports.*"
  Delete "$INSTDIR\PPortJoy*.*"
  Delete "$INSTDIR\PPJoyBus*.*"

  ; Remove the program directory
  RMDir "$INSTDIR"

  ; Remove uninstall registry entry.
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  
  SetAutoClose true
SectionEnd
