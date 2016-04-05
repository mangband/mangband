
; NSIS Installer for Windows Mangband Server 
; Downlad NSIS at nsis.sourceforge.net

; You can change this if you've moved the source or this file.
!define DEV_DIR "..\..\"

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "MAngband Server"
!define PRODUCT_VERSION "1.1.3"
!define VER "113"
!define PRODUCT_PUBLISHER ""
!define PRODUCT_WEB_SITE "http://mangband.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\MAngband.exe"
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
OutFile "mangband-server-setup-${VER}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails nevershow
ShowUnInstDetails nevershow

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "${DEV_DIR}MAngband.exe"
  File "README.htm"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Start Server.lnk" "$INSTDIR\run-server.bat"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\ReadMe.lnk" "$INSTDIR\README.htm"
  ;CreateShortCut "$DESKTOP\Start MAngband Server.lnk" "$INSTDIR\run-server.bat"

  CreateDirectory "$INSTDIR\lib"
  CreateDirectory "$INSTDIR\lib\file"
  CreateDirectory "$INSTDIR\lib\save"
  File "run-server.bat"
  SetOverwrite off
  File "${DEV_DIR}mangband.cfg"
  SetOverwrite ifnewer
  SetOutPath "$INSTDIR\lib"
  FILE /r /x .svn "${DEV_DIR}lib\data"
  FILE /r /x .svn "${DEV_DIR}lib\edit"
  FILE /r /x .svn "${DEV_DIR}lib\help"
  FILE /r /x .svn "${DEV_DIR}lib\text"
  FILE /r /x .svn "${DEV_DIR}lib\user"

SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\MAngband.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\MAngband.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

  ExecShell "open" "$INSTDIR\readme.htm"  

SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "The ${PRODUCT_NAME} was successfully removed from your computer."
FunctionEnd

Function .onInit

FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove the ${PRODUCT_NAME} and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\MAngband.exe"
  Delete "$INSTDIR\exchndl.dll"
  Delete "$INSTDIR\MAngband.cfg"

  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
