; installer.nsi
;
; This script is based on example2.nsi and customised for Mscgen

;--------------------------------

!include "EnvVarUpdate.nsh"
!include "FileFunc.nsh"

;--------------------------------

; The name of the installer
Name Mscgen

!define VERSION dev

!define FILES binstage\w32\mscgen-${VERSION}

; The file to write
OutFile "mscgen_${VERSION}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Mscgen

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Mscgen" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Show what's happening by default
ShowInstDetails show

; LZMA is cool
SetCompressor /SOLID /FINAL lzma


;--------------------------------

; Pages
Page license
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles


;--------------------------------

LicenseText "Mscgen is licensed under the GPLv2.  Please review this licence displayed below or check the COPYING file after installation." "Next >"
LicenseData ${FILES}\share\doc\mscgen\COPYING

CompletedText "Completed!"

;--------------------------------

; The stuff to install
Section "Mscgen (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  File "${FILES}\bin\mscgen.exe"
  File "${FILES}\bin\bgd.dll"

  File "${FILES}\share\doc\mscgen\COPYING"
  File "${FILES}\share\doc\mscgen\README"
  File "${FILES}\share\doc\mscgen\ChangeLog"

  ; Add example MSCs
  SetOutPath $INSTDIR\examples
  File "${FILES}\share\doc\mscgen\examples\*.msc"

  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR" 

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Mscgen "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "DisplayName" "Mscgen"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "Publisher" "Michael McTernan"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "HelpLink" "http://www.mcternan.me.uk/mscgen"

  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2 
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "EstimatedSize" "$0"   
  
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /S'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

SectionEnd


;--------------------------------

; Uninstaller

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mscgen"
  DeleteRegKey HKLM SOFTWARE\Mscgen

  ; Remove files and uninstaller
  Delete $INSTDIR
  Delete $INSTDIR\mscgen.exe
  Delete $INSTDIR\bgd.dll
  Delete $INSTDIR\COPYING
  Delete $INSTDIR\README
  Delete $INSTDIR\ChangeLog
  Delete $INSTDIR\examples\*.msc
  Delete $INSTDIR\uninstall.exe
  RMDir  $INSTDIR\examples
  RMDir  $INSTDIR

  ; Remove directories used
  RMDir "$INSTDIR"
  
  ; Remove from path
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR"

SectionEnd


; END OF FILE

