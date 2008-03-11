
; NSIS Installer for Windows Mangband Client 
; Originally by Graham R King (4th November 2005)
; Downlad NSIS at nsis.sourceforge.net

; You will need to change this...
!define DEV_DIR "C:\Projects\Eclipse\Mangband\"

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "MAngband"
!define PRODUCT_VERSION "0.7.2 (build 3)"
!define VER "072-build3"
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
OutFile "MAngband-client-setup-${VER}.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails nevershow
ShowUnInstDetails nevershow

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "${DEV_DIR}mangclient.exe"
  File "${DEV_DIR}README-client.htm"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\MAngband.lnk" "$INSTDIR\mangclient.exe"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\ReadMe.lnk" "$INSTDIR\README-client.htm"

  CreateDirectory "$INSTDIR\lib"
  CreateDirectory "$INSTDIR\lib\data"
  CreateDirectory "$INSTDIR\lib\file"
  CreateDirectory "$INSTDIR\lib\save"
  File "${DEV_DIR}mangclient.ini"
  SetOutPath "$INSTDIR\lib"
  FILE /r "${DEV_DIR}lib\game"
  FILE /r "${DEV_DIR}lib\help"
  FILE /r "${DEV_DIR}lib\text"
  FILE /r "${DEV_DIR}lib\user"
  FILE /r "${DEV_DIR}lib\xtra"

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

Function .onInit

FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove the ${PRODUCT_NAME} and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\mangclient.exe"
  Delete "$INSTDIR\mangclient.ini"

  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir /r "$INSTDIR\lib"
  RMDir /r "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
