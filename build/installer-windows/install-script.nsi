!include "MUI2.nsh"
!define MUI_ICON "icon.ico"
!define MUI_UNICON "icon.ico"

Name "x-engine"
OutFile "x-engine-installer.exe"
InstallDir "$APPDATA\Local\x-engine"

RequestExecutionLevel user
SetCompressor /SOLID /FINAL lzma
SilentInstall normal

!insertmacro MUI_PAGE_WELCOME
Page directory
Page instfiles

Section "Install"
  SetOutPath "$INSTDIR"
  RMDir /r "$INSTDIR"
  File "x-engine.exe"
  File /r "games"
  File /r "tools"
  File "icon.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\x-engine"
  CreateShortcut "$SMPROGRAMS\x-engine\x-engine.lnk" "$INSTDIR\x-engine.exe" ""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "InstDir" '"$INSTDIR"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "DisplayName" "x-engine"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "DisplayIcon" '"$INSTDIR\icon.ico"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "DisplayVersion" "1"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "Publisher" "The x-engine Developers"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "Software\Classes\.xengine" "" "x-engine.project"
  WriteRegStr HKCU "Software\Classes\x-engine.project" "" "x-engine Project"
  WriteRegStr HKCU "Software\Classes\x-engine.project\DefaultIcon" "" "$INSTDIR\x-engine.exe"
  WriteRegStr HKCU "Software\Classes\x-engine.project\Shell\open\command" "" '"$INSTDIR\x-engine.exe" "%1"'
  SetShellVarContext current
  CreateShortCut "$DESKTOP\x-engine.lnk" "$INSTDIR\x-engine.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\x-engine.exe"
  Delete "$INSTDIR\games"
  Delete "$INSTDIR\tools"
  Delete "$SMPROGRAMS\x-engine\x-engine.lnk"
  RMDir "$SMPROGRAMS\x-engine"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\x-engine"
  Delete "$DESKTOP\x-engine.lnk"
  DeleteRegKey HKCU "Software\Classes\.xengine"
  DeleteRegKey HKCU "Software\Classes\x-engine.project"
  DeleteRegKey HKCU "Software\Classes\x-engine.project\DefaultIcon"
  DeleteRegKey HKCU "Software\Classes\x-engine.project\Shell\open\command"
SectionEnd

Function .OnInstSuccess
  Exec "$INSTDIR\x-engine.exe"
FunctionEnd

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"
