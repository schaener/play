!include "MUI2.nsh"

!searchparse /file ../Source/AppDef.h '#define APP_VERSIONSTR _T("' APP_VERSION '")'

; The name of the installer
Name "PsfPlayer v${APP_VERSION}"

; The file to write
OutFile "PsfPlayer-${APP_VERSION}-32.exe"

; The default installation directory
InstallDir $PROGRAMFILES\PsfPlayer

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_PsfPlayer" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
!insertmacro MUI_LANGUAGE "English"
;--------------------------------

; Pages

;Page components
;Page directory
;Page instfiles

;UninstPage uninstConfirm
;UninstPage instfiles

;--------------------------------

!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\PsfPlayer"

; The stuff to install
Section "PsfPlayer (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\..\..\build\tools\PsfPlayer\Source\ui_win32\Release\PsfPlayer.exe"
  File "..\..\..\build\tools\PsfPlayer\Source\ui_win32\Release\SH_WaveOut.dll"
  File "..\..\..\build\tools\PsfPlayer\Source\ui_win32\Release\SH_XAudio2.dll"
  File "..\..\..\build\tools\PsfPlayer\Source\ui_win32\Release\TDEmu.dll"
  File "..\changelog.html"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_PsfPlayer "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayName" "PsfPlayer"
  WriteRegStr HKLM "${REG_UNINSTALL}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "${REG_UNINSTALL}" "NoModify" 1
  WriteRegDWORD HKLM "${REG_UNINSTALL}" "NoRepair" 1
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayIcon" '"$INSTDIR\PsfPlayer.exe"'
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayVersion" "${APP_VERSION}"
  WriteUninstaller "uninstall.exe"
  
SectionEnd

!include "..\..\..\installer_win32\vcredist2015_x86.nsh"

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\PsfPlayer"
  CreateShortCut "$SMPROGRAMS\PsfPlayer\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\PsfPlayer\PsfPlayer.lnk" "$INSTDIR\PsfPlayer.exe" "" "$INSTDIR\PsfPlayer.exe" 0
  CreateShortCut "$SMPROGRAMS\PsfPlayer\changelog.html.lnk" "$INSTDIR\changelog.html" "" "$INSTDIR\changelog.html" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "${REG_UNINSTALL}"
  DeleteRegKey HKLM SOFTWARE\NSIS_PsfPlayer

  ; Remove files and uninstaller
  Delete $INSTDIR\PsfPlayer.exe
  Delete $INSTDIR\SH_WaveOut.dll
  Delete $INSTDIR\SH_XAudio2.dll
  Delete $INSTDIR\TDEmu.dll
  Delete $INSTDIR\changelog.html
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\PsfPlayer\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\PsfPlayer"
  RMDir "$INSTDIR"

SectionEnd
