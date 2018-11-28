
; NSIS Installer for Windows Mangband Client 
; Downlad NSIS at nsis.sourceforge.net

; Where is the root of the project?
!define DEV_DIR "..\..\"
!include "FileFunc.nsh"
!insertmacro GetParent

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "MAngband"
!define PRODUCT_VERSION "1.1.3"
!define VER "113"
!define PRODUCT_PUBLISHER ""
!define PRODUCT_WEB_SITE "http://mangband.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\MAngclient.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
CRCCheck Off

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
;!define MUI_FINISHPAGE_RUN "$INSTDIR\README.htm"
!define MUI_FINISHPAGE_LINK "Read the README file for info on how to get started"
!define MUI_FINISHPAGE_LINK_LOCATION "$INSTDIR\README.htm"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "mangband-client-setup-${VER}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails nevershow
ShowUnInstDetails nevershow

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "${DEV_DIR}mangclient.exe"
  File "${DEV_DIR}mangclient-sdl.exe"
  File "${DEV_DIR}sdl.dll"
  File "README-client.htm"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\MAngband.lnk" "$INSTDIR\mangclient.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\ReadMe.lnk" "$INSTDIR\README-client.htm"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\MAngband (One Window).lnk" "$INSTDIR\mangclient-sdl.exe"

  CreateDirectory "$INSTDIR\lib"
  CreateDirectory "$INSTDIR\lib\file"
  CreateDirectory "$INSTDIR\lib\help"
  CreateDirectory "$INSTDIR\tmp"
  SetOverwrite off
  File "${DEV_DIR}mangclient.ini"
  SetOverwrite ifnewer
  SetOutPath "$INSTDIR\lib"
  FILE /r /x .svn "${DEV_DIR}lib\user"
  SetOverwrite try
  FILE /r /x .svn "${DEV_DIR}lib\xtra"
  SetOverwrite ifnewer

SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\mangclient.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\mangclient.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

  ExecShell "open" "$INSTDIR\readme-client.htm"  

SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "The ${PRODUCT_NAME} was successfully removed from your computer."
FunctionEnd

; This removes a previous beta if it's installed
Function .onInit

  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\MAngband Beta" \
  "UninstallString"
  StrCmp $R0 "" done foundold

foundold:
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "A Beta version of MAngband is already installed and should be removed. Click `OK` to remove \
  the beta version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort

;Run the uninstaller
uninst:
  ClearErrors

  ${GetParent} $R0 $R9

  ExecWait '$R0 _?=$R9' 
;  ExecWait '$R0' 

  IfErrors no_remove_uninstaller
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:

done:

FunctionEnd


Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove the ${PRODUCT_NAME} and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\mangclient.exe"
  Delete "$INSTDIR\mangclient-sdl.exe"
  Delete "$INSTDIR\sdl.dll"
  Delete "$INSTDIR\mangclient.ini"

  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
