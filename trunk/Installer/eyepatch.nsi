; eyepatch.nsi

;--------------------------------

Name "Eyepatch Installer"
Icon "eyepatch-install.ico"

OutFile "EyepatchInstall.exe"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
; InstallColors FF8080 000030
XPStyle on

; The default installation directory
InstallDir $PROGRAMFILES\Eyepatch

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Eyepatch" "Install_Dir"

CheckBitmap "${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Eyepatch (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put files there
  File "cv100.dll"
  File "cvaux100.dll"
  File "cxcore100.dll"
  File "highgui100.dll"
  File "libguide40.dll"
  File "pthreadGC2.dll"
  File "swscale-0.dll"
  File "avcodec-51.dll"
  File "avformat-52.dll"
  File "avutil-49.dll"
  File "libgslcblas.dll"
  File "libgsl.dll"
  File "tessdll.dll"
  File "Eyepatch.exe"

  ; Copy tesseract data files
  CreateDirectory "$INSTDIR\tessdata"
  SetOutPath "$INSTDIR\tessdata"
  File "tessdata\eng.DangAmbigs"
  File "tessdata\eng.freq-dawg"
  File "tessdata\eng.inttemp"
  File "tessdata\eng.normproto"
  File "tessdata\eng.pffmtable"
  File "tessdata\eng.unicharset"
  File "tessdata\eng.user-words"
  File "tessdata\eng.word-dawg"

  CreateDirectory "$INSTDIR\tessdata\configs"
  SetOutPath "$INSTDIR\tessdata\configs"
  File "tessdata\configs\api_config"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Eyepatch "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Eyepatch" "DisplayName" "Eyepatch"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Eyepatch" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Eyepatch" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Eyepatch" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Microsoft Visual C++ 8.0 DLLs (Required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Install VC redistributables
  File "vcredist_x86.exe"
  ExecWait '"$INSTDIR\vcredist_x86.exe" /q'
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Divx Codec (Required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Install Divx Codec
  File "DivXInstaller.exe"
  ExecWait '"$INSTDIR\DivXInstaller.exe"'
  
SectionEnd


; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Eyepatch"
  CreateShortCut "$SMPROGRAMS\Eyepatch\Uninstall.lnk" "$INSTDIR\uninstall.exe" ""  "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Eyepatch\Eyepatch.lnk" "$INSTDIR\Eyepatch.exe" "" "$INSTDIR\Eyepatch.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

UninstallText "This will completely remove Eyepatch and all of its components."
UninstallIcon "eyepatch-uninstall.ico"

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Eyepatch"
  DeleteRegKey HKLM SOFTWARE\Eyepatch

  ; Remove files and uninstaller

  Delete $INSTDIR\cv100.dll
  Delete $INSTDIR\cvaux100.dll
  Delete $INSTDIR\cxcore100.dll
  Delete $INSTDIR\highgui100.dll
  Delete $INSTDIR\libguide40.dll
  Delete $INSTDIR\vcredist_x86.exe
  Delete $INSTDIR\pthreadGC2.dll
  Delete $INSTDIR\swscale-0.dll
  Delete $INSTDIR\avcodec-51.dll
  Delete $INSTDIR\avformat-51.dll
  Delete $INSTDIR\avutil-49.dll
  Delete $INSTDIR\DivXInstaller.exe
  Delete $INSTDIR\libgslcblas.dll
  Delete $INSTDIR\libgsl.dll
  Delete $INSTDIR\tessdll.dll
  Delete $INSTDIR\Eyepatch.exe
  Delete $INSTDIR\uninstall.exe

  Delete "$INSTDIR\tessdata\configs\*.*"
  Delete "$INSTDIR\tessdata\*.*"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Eyepatch\*.*"
  RMDir "$SMPROGRAMS\Eyepatch"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Eyepatch"
  RMDir "$INSTDIR\tessdata\configs"
  RMDir "$INSTDIR\tessdata"
  RMDir "$INSTDIR"

SectionEnd
