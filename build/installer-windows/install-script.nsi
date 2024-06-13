!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "Polaris Engine"
OutFile "polaris-engine-installer.exe"
InstallDir "$APPDATA\Local\polaris-engine"

RequestExecutionLevel user
SetCompressor /SOLID /FINAL lzma
SilentInstall normal

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  RMDir /r "$INSTDIR"
  File "polaris-engine.exe"
  File /r "games"
  File /r "tools"
  File "icon.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\polaris-engine"
  CreateShortcut "$SMPROGRAMS\polaris-engine\polaris-engine.lnk" "$INSTDIR\polaris-engine.exe" ""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "InstDir" '"$INSTDIR"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "DisplayName" "polaris-engine"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "DisplayIcon" '"$INSTDIR\icon.ico"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "DisplayVersion" "1"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "Publisher" "The polaris-engine Developers"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "Software\Classes\.polaris" "" "polaris-engine.project"
  WriteRegStr HKCU "Software\Classes\polaris-engine.project" "" "polaris-engine Project"
  WriteRegStr HKCU "Software\Classes\polaris-engine.project\DefaultIcon" "" "$INSTDIR\polaris-engine.exe"
  WriteRegStr HKCU "Software\Classes\polaris-engine.project\Shell\open\command" "" '"$INSTDIR\polaris-engine.exe" "%1"'
  SetShellVarContext current
  CreateShortCut "$DESKTOP\polaris-engine.lnk" "$INSTDIR\polaris-engine.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\polaris-engine.exe"
  Delete "$INSTDIR\games"
  Delete "$INSTDIR\tools"
  Delete "$SMPROGRAMS\polaris-engine\polaris-engine.lnk"
  RMDir "$SMPROGRAMS\polaris-engine"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\polaris-engine"
  Delete "$DESKTOP\polaris-engine.lnk"
  DeleteRegKey HKCU "Software\Classes\.polaris"
  DeleteRegKey HKCU "Software\Classes\polaris-engine.project"
  DeleteRegKey HKCU "Software\Classes\polaris-engine.project\DefaultIcon"
  DeleteRegKey HKCU "Software\Classes\polaris-engine.project\Shell\open\command"
SectionEnd

Function .OnInstSuccess
  Exec "$INSTDIR\polaris-engine.exe"
FunctionEnd

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"
