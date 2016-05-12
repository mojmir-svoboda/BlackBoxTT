# This is install script for NSIS installer for BlackBox 4 Windows

# where to create install.exe
OutFile c:\_builds\install.exe

# where to install program
InstallDir c:\bbZero

# good for debugging
ShowInstDetails Show

!include LogicLib.nsh
!include WinVer.nsh
!include nsDialogs.nsh
!include x64.nsh
!include Sections.nsh
!addplugindir .

Var OptConfigs
Var OptStyles
Var OptCorePlugins
Var OptPlugins
Var OptPlugins2

# Set the text to prompt user to enter a directory
DirText "This will install BlackBox 4 Windows program on your computer. Choose a directory"

Name "BlackBox 4 Windows"
RequestExecutionLevel admin
AddBrandingImage left 256

Page Custom brandimage "" ": Brand Image"
Page License
Page Directory
Page Components
Page Custom windetectionPageEnter windetectionPageLeave
Page InstFiles
Page Custom shellPageEnter shellPageLeave
#UninstPage uninstConfirm
#UninstPage instfiles

LicenseData "GPL.txt" 

Function brandimage
  SetOutPath "$TEMP"
  SetFileAttributes installer.bmp temporary
  File installer.bmp
  SetBrandingImage "$TEMP\installer.bmp" /resizetofit
FunctionEnd

# build selection variables
Var win_xp
Var win_64
var BuildVerDialog
var Group1BuildVerRadioXP
var Group1BuildVerRadioVista
var Group1BuildVer2Radio32
var Group1BuildVer2Radio64
Var GroupBox1
Var GroupBox2
# 'as shell' variables
var as_shell
var curr_user
var ShellDialog
Var GroupBox3
Var GroupBox4
var RadioButtonAsShell
var RadioButtonNoShell
var RadioButtonCurrUser
var RadioButtonAllUser
 
# Dummy section visible, RO means read only, user can't
# change this. This should remain empty.
Section "Required Files"
  SectionIn RO
SectionEnd

# Visible options for the user
Section "Configurations" SecConfigs
SectionEnd
Section "Styles" SecStyles
SectionEnd
Section "Core plugins" SecCorePlugins
SectionEnd
Section "Optional plugins I." SecPlugins
SectionEnd
Section "Optional plugins II." SecPlugins2
SectionEnd


# Invisible section
Section "-ReadOptions"
  # This is where we read the optional sections to see if
  # they are selected, and set our variables to reflect this

  SectionGetFlags ${SecConfigs} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +3 +3
    StrCpy $OptConfigs 1
    GoTo +2
  StrCpy $OptConfigs 0

  SectionGetFlags ${SecStyles} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +3 +3
    StrCpy $OptStyles 1
    GoTo +2
  StrCpy $OptStyles 0

  SectionGetFlags ${SecCorePlugins} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +3 +3
    StrCpy $OptCorePlugins 1
    GoTo +2
  StrCpy $OptCorePlugins 0

  SectionGetFlags ${SecPlugins} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +3 +3
    StrCpy $OptPlugins 1
    GoTo +2
  StrCpy $OptPlugins 0

  SectionGetFlags ${SecPlugins2} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +3 +3
    StrCpy $OptPlugins2 1
    GoTo +2
  StrCpy $OptPlugins2 0
SectionEnd

################################# XP 32b ######################################
Section /o "-XP_32" Sec_XP_32
  DetailPrint "XP 32 bit Required Files"

  SetOutPath $INSTDIR
  File "vs_xp_32\bbnote.exe"
  File "vs_xp_32\bbnote-proxy.dll"
  File "vs_xp_32\bbstylemaker.exe"
  File "vs_xp_32\blackbox.exe"
  File "vs_xp_32\bsetbg.exe"
  File "vs_xp_32\bsetroot.exe"
  File "vs_xp_32\bsetshell.exe"
  File "vs_xp_32\deskhook.dll"
  File "vs_xp_32\readme.txt"
 
  StrCmp $OptConfigs 0 SkipXP32_1
  DetailPrint "XP 32 bit Optional Configs"

  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
  File "vs_xp_32\blackbox.rc"
  File "vs_xp_32\bsetroot.rc"
  File "vs_xp_32\extensions.rc"
  File "vs_xp_32\menu.rc"
  File "vs_xp_32\plugins.rc"
  File "vs_xp_32\shellfolders.rc"
  File "vs_xp_32\stickywindows.ini"

SkipXP32_1:

  StrCmp $OptStyles 0 SkipXP32_2
  DetailPrint "XP 32 bit Optional Styles"
 
  SetOutPath $INSTDIR\backgrounds
  File /r "vs_xp_32\backgrounds\"
  SetOutPath $INSTDIR\styles
  File /r "vs_xp_32\styles\"

SkipXP32_2:

  StrCmp $OptCorePlugins 0 SkipXP32_3
  DetailPrint "XP 32 bit Optional Core Plugins"

  #SetOutPath $INSTDIR\plugins\bbAnalog
	#File /r "vs_xp_32\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbColor3dc
	File /r "vs_xp_32\plugins\bbColor3dc\"
  SetOutPath $INSTDIR\plugins\bbIconBox
	File /r "vs_xp_32\plugins\bbIconBox\"
  #SetOutPath $INSTDIR\plugins\bbInterface
	#File /r "vs_xp_32\plugins\bbInterface\"
  SetOutPath $INSTDIR\plugins\bbKeys
	File /r "vs_xp_32\plugins\bbKeys\"
  SetOutPath $INSTDIR\plugins\BlackboxBar
	File /r "vs_xp_32\plugins\BlackboxBar\"
  SetOutPath $INSTDIR\plugins\bbLeanSkin
	File /r "vs_xp_32\plugins\bbLeanSkin\"
  SetOutPath $INSTDIR\plugins\bbSlit
	File /r "vs_xp_32\plugins\bbSlit\"

SkipXP32_3:

  StrCmp $OptPlugins 0 SkipXP32_4
  DetailPrint "XP 32 bit Optional Plugins 1"

  SetOutPath $INSTDIR\plugins\bbAnalogExMod
	File /r "vs_xp_32\plugins\bbAnalogExMod\"
  SetOutPath $INSTDIR\plugins\bbAnalog
	File /r "vs_xp_32\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbCalendar
	File /r "vs_xp_32\plugins\bbCalendar\"
  SetOutPath $INSTDIR\plugins\BBDigitalEx
	File /r "vs_xp_32\plugins\BBDigitalEx\"
  SetOutPath $INSTDIR\plugins\bbFooMan
	File /r "vs_xp_32\plugins\bbFooMan\"
  SetOutPath $INSTDIR\plugins\bbMuse
	File /r "vs_xp_32\plugins\bbMuse\"
  SetOutPath $INSTDIR\plugins\bbPlayer
	File /r "vs_xp_32\plugins\bbPlayer\"
  SetOutPath $INSTDIR\plugins\bbSeekBar
	File /r "vs_xp_32\plugins\bbSeekBar\"
  SetOutPath $INSTDIR\plugins\bbFoomp
	File /r "vs_xp_32\plugins\bbFoomp\"
  SetOutPath $INSTDIR\plugins\BBMagnify
	File /r "vs_xp_32\plugins\BBMagnify\"
  SetOutPath $INSTDIR\plugins\bbPager
	File /r "vs_xp_32\plugins\bbPager\"
  SetOutPath $INSTDIR\plugins\bbRecycleBin
	File /r "vs_xp_32\plugins\bbRecycleBin\"
  SetOutPath $INSTDIR\plugins\BBRSS
	File /r "vs_xp_32\plugins\BBRSS\"
  SetOutPath $INSTDIR\plugins\BBStyle
	File /r "vs_xp_32\plugins\BBStyle\"
  SetOutPath $INSTDIR\plugins\BBSysMeter
	File /r "vs_xp_32\plugins\BBSysMeter\"
  SetOutPath $INSTDIR\plugins\bbWorkspaceWheel
	File /r "vs_xp_32\plugins\bbWorkspaceWheel\"
  SetOutPath $INSTDIR\plugins\SystemBarEx
	File /r "vs_xp_32\plugins\SystemBarEx\"

SkipXP32_4:

  StrCmp $OptPlugins2 0 SkipXP32_5
  DetailPrint "XP 32 bit Optional Plugins 2"

  SetOutPath $INSTDIR\plugins\BB8Ball
	File /r "vs_xp_32\plugins\BB8Ball\"
  SetOutPath $INSTDIR\plugins\bbInterface_iTunes
	File /r "vs_xp_32\plugins\bbInterface_iTunes\"
  SetOutPath $INSTDIR\plugins\BBMessageBox
	File /r "vs_xp_32\plugins\BBMessageBox\"
  SetOutPath $INSTDIR\plugins\BBXO
	File /r "vs_xp_32\plugins\BBXO\"

SkipXP32_5:
SectionEnd

################################# XP 64b ######################################
Section /o "-XP_64" Sec_XP_64
  DetailPrint "XP 64 bit Required Files"

  SetOutPath $INSTDIR
  File "vs_xp_64\bbnote.exe"
  File "vs_xp_64\bbnote-proxy.dll"
  File "vs_xp_64\bbstylemaker.exe"
  File "vs_xp_64\blackbox.exe"
  File "vs_xp_64\bsetbg.exe"
  File "vs_xp_64\bsetroot.exe"
  File "vs_xp_64\bsetshell.exe"
  File "vs_xp_64\deskhook.dll"
  File "vs_xp_64\readme.txt"
 
  StrCmp $OptConfigs 0 SkipXP64_1
  DetailPrint "XP 64 bit Optional Configs"

  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
  File "vs_xp_64\blackbox.rc"
  File "vs_xp_64\bsetroot.rc"
  File "vs_xp_64\extensions.rc"
  File "vs_xp_64\menu.rc"
  File "vs_xp_64\plugins.rc"
  File "vs_xp_64\shellfolders.rc"
  File "vs_xp_64\stickywindows.ini"

SkipXP64_1:

  StrCmp $OptStyles 0 SkipXP64_2
  DetailPrint "XP 64 bit Optional Styles"
 
  SetOutPath $INSTDIR\backgrounds
  File /r "vs_xp_64\backgrounds\"
  SetOutPath $INSTDIR\styles
  File /r "vs_xp_64\styles\"

SkipXP64_2:

  StrCmp $OptCorePlugins 0 SkipXP64_3
  DetailPrint "XP 64 bit Optional Core Plugins"

  #SetOutPath $INSTDIR\plugins\bbAnalog
	#File /r "vs_xp_64\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbColor3dc
	File /r "vs_xp_64\plugins\bbColor3dc\"
  SetOutPath $INSTDIR\plugins\bbIconBox
	File /r "vs_xp_64\plugins\bbIconBox\"
  #SetOutPath $INSTDIR\plugins\bbInterface
	#File /r "vs_xp_64\plugins\bbInterface\"
  SetOutPath $INSTDIR\plugins\bbKeys
	File /r "vs_xp_64\plugins\bbKeys\"
  SetOutPath $INSTDIR\plugins\BlackboxBar
	File /r "vs_xp_64\plugins\BlackboxBar\"
  SetOutPath $INSTDIR\plugins\bbLeanSkin
	File /r "vs_xp_64\plugins\bbLeanSkin\"
  SetOutPath $INSTDIR\plugins\bbSlit
	File /r "vs_xp_64\plugins\bbSlit\"

SkipXP64_3:

  StrCmp $OptPlugins 0 SkipXP64_4
  DetailPrint "XP 64 bit Optional Plugins 1"

  SetOutPath $INSTDIR\plugins\bbAnalogExMod
	File /r "vs_xp_64\plugins\bbAnalogExMod\"
  SetOutPath $INSTDIR\plugins\bbAnalog
	File /r "vs_xp_64\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbCalendar
	File /r "vs_xp_64\plugins\bbCalendar\"
  SetOutPath $INSTDIR\plugins\BBDigitalEx
	File /r "vs_xp_64\plugins\BBDigitalEx\"
  SetOutPath $INSTDIR\plugins\bbFooMan
	File /r "vs_xp_64\plugins\bbFooMan\"
  SetOutPath $INSTDIR\plugins\bbMuse
	File /r "vs_xp_64\plugins\bbMuse\"
  SetOutPath $INSTDIR\plugins\bbPlayer
	File /r "vs_xp_64\plugins\bbPlayer\"
  SetOutPath $INSTDIR\plugins\bbSeekBar
	File /r "vs_xp_64\plugins\bbSeekBar\"
  SetOutPath $INSTDIR\plugins\bbFoomp
	File /r "vs_xp_64\plugins\bbFoomp\"
  SetOutPath $INSTDIR\plugins\BBMagnify
	File /r "vs_xp_64\plugins\BBMagnify\"
  SetOutPath $INSTDIR\plugins\bbPager
	File /r "vs_xp_64\plugins\bbPager\"
  SetOutPath $INSTDIR\plugins\bbRecycleBin
	File /r "vs_xp_64\plugins\bbRecycleBin\"
  SetOutPath $INSTDIR\plugins\BBRSS
	File /r "vs_xp_64\plugins\BBRSS\"
  SetOutPath $INSTDIR\plugins\BBStyle
	File /r "vs_xp_64\plugins\BBStyle\"
  SetOutPath $INSTDIR\plugins\BBSysMeter
	File /r "vs_xp_64\plugins\BBSysMeter\"
  SetOutPath $INSTDIR\plugins\bbWorkspaceWheel
	File /r "vs_xp_64\plugins\bbWorkspaceWheel\"
  SetOutPath $INSTDIR\plugins\SystemBarEx
	File /r "vs_xp_64\plugins\SystemBarEx\"

SkipXP64_4:

  StrCmp $OptPlugins2 0 SkipXP64_5
  DetailPrint "XP 64 bit Optional Plugins 2"

  SetOutPath $INSTDIR\plugins\BB8Ball
	File /r "vs_xp_64\plugins\BB8Ball\"
  SetOutPath $INSTDIR\plugins\bbInterface_iTunes
	File /r "vs_xp_64\plugins\bbInterface_iTunes\"
  SetOutPath $INSTDIR\plugins\BBMessageBox
	File /r "vs_xp_64\plugins\BBMessageBox\"
  SetOutPath $INSTDIR\plugins\BBXO
	File /r "vs_xp_64\plugins\BBXO\"

SkipXP64_5:

SectionEnd

################################ Vista 32b ####################################
Section /o "-Vista_32" Sec_Vista_32

  DetailPrint "Vista 32 bit Required Files"

  SetOutPath $INSTDIR
  File "vs_vista_32\bbnote.exe"
  File "vs_vista_32\bbnote-proxy.dll"
  File "vs_vista_32\bbstylemaker.exe"
  File "vs_vista_32\blackbox.exe"
  File "vs_vista_32\bsetbg.exe"
  File "vs_vista_32\bsetroot.exe"
  File "vs_vista_32\bsetshell.exe"
  File "vs_vista_32\deskhook.dll"
  File "vs_vista_32\readme.txt"
 
  StrCmp $OptConfigs 0 SkipVista32_1
  DetailPrint "Vista 32 bit Optional Configs"

  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
  File "vs_vista_32\blackbox.rc"
  File "vs_vista_32\bsetroot.rc"
  File "vs_vista_32\extensions.rc"
  File "vs_vista_32\menu.rc"
  File "vs_vista_32\plugins.rc"
  File "vs_vista_32\shellfolders.rc"
  File "vs_vista_32\stickywindows.ini"

SkipVista32_1:

  StrCmp $OptStyles 0 SkipVista32_2
  DetailPrint "Vista 32 bit Optional Styles"
 
  SetOutPath $INSTDIR\backgrounds
  File /r "vs_vista_32\backgrounds\"
  SetOutPath $INSTDIR\styles
  File /r "vs_vista_32\styles\"

SkipVista32_2:

  StrCmp $OptCorePlugins 0 SkipVista32_3
  DetailPrint "Vista 32 bit Optional Core Plugins"

  #SetOutPath $INSTDIR\plugins\bbAnalog
	#File /r "vs_vista_32\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbColor3dc
	File /r "vs_vista_32\plugins\bbColor3dc\"
  SetOutPath $INSTDIR\plugins\bbIconBox
	File /r "vs_vista_32\plugins\bbIconBox\"
  #SetOutPath $INSTDIR\plugins\bbInterface
	#File /r "vs_vista_32\plugins\bbInterface\"
  SetOutPath $INSTDIR\plugins\bbKeys
	File /r "vs_vista_32\plugins\bbKeys\"
  SetOutPath $INSTDIR\plugins\BlackboxBar
	File /r "vs_vista_32\plugins\BlackboxBar\"
  SetOutPath $INSTDIR\plugins\bbLeanSkin
	File /r "vs_vista_32\plugins\bbLeanSkin\"
  SetOutPath $INSTDIR\plugins\bbSlit
	File /r "vs_vista_32\plugins\bbSlit\"

SkipVista32_3:

  StrCmp $OptPlugins 0 SkipVista32_4
  DetailPrint "Vista 32 bit Optional Plugins 1"

  SetOutPath $INSTDIR\plugins\bbAnalogExMod
	File /r "vs_vista_32\plugins\bbAnalogExMod\"
  SetOutPath $INSTDIR\plugins\bbAnalog
	File /r "vs_vista_32\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbCalendar
	File /r "vs_vista_32\plugins\bbCalendar\"
  SetOutPath $INSTDIR\plugins\BBDigitalEx
	File /r "vs_vista_32\plugins\BBDigitalEx\"
  SetOutPath $INSTDIR\plugins\bbFooMan
	File /r "vs_vista_32\plugins\bbFooMan\"
  SetOutPath $INSTDIR\plugins\bbMuse
	File /r "vs_vista_32\plugins\bbMuse\"
  SetOutPath $INSTDIR\plugins\bbPlayer
	File /r "vs_vista_32\plugins\bbPlayer\"
  SetOutPath $INSTDIR\plugins\bbSeekBar
	File /r "vs_vista_32\plugins\bbSeekBar\"
  SetOutPath $INSTDIR\plugins\bbFoomp
	File /r "vs_vista_32\plugins\bbFoomp\"
  SetOutPath $INSTDIR\plugins\BBMagnify
	File /r "vs_vista_32\plugins\BBMagnify\"
  SetOutPath $INSTDIR\plugins\bbPager
	File /r "vs_vista_32\plugins\bbPager\"
  SetOutPath $INSTDIR\plugins\bbRecycleBin
	File /r "vs_vista_32\plugins\bbRecycleBin\"
  SetOutPath $INSTDIR\plugins\BBRSS
	File /r "vs_vista_32\plugins\BBRSS\"
  SetOutPath $INSTDIR\plugins\BBStyle
	File /r "vs_vista_32\plugins\BBStyle\"
  SetOutPath $INSTDIR\plugins\BBSysMeter
	File /r "vs_vista_32\plugins\BBSysMeter\"
  SetOutPath $INSTDIR\plugins\bbWorkspaceWheel
	File /r "vs_vista_32\plugins\bbWorkspaceWheel\"
  SetOutPath $INSTDIR\plugins\SystemBarEx
	File /r "vs_vista_32\plugins\SystemBarEx\"

SkipVista32_4:

  StrCmp $OptPlugins2 0 SkipVista32_5
  DetailPrint "Vista 32 bit Optional Plugins 2"

  SetOutPath $INSTDIR\plugins\BB8Ball
	File /r "vs_vista_32\plugins\BB8Ball\"
  SetOutPath $INSTDIR\plugins\bbInterface_iTunes
	File /r "vs_vista_32\plugins\bbInterface_iTunes\"
  SetOutPath $INSTDIR\plugins\BBMessageBox
	File /r "vs_vista_32\plugins\BBMessageBox\"
  SetOutPath $INSTDIR\plugins\BBXO
	File /r "vs_vista_32\plugins\BBXO\"

SkipVista32_5:

SectionEnd

################################ Vista 64b ####################################
Section /o "-Vista_64" Sec_Vista_64

  DetailPrint "Vista 64 bit Required Files"

  SetOutPath $INSTDIR
  File "vs_vista_64\bbnote.exe"
  File "vs_vista_64\bbnote-proxy.dll"
  File "vs_vista_64\bbstylemaker.exe"
  File "vs_vista_64\blackbox.exe"
  File "vs_vista_64\bsetbg.exe"
  File "vs_vista_64\bsetroot.exe"
  File "vs_vista_64\bsetshell.exe"
  File "vs_vista_64\deskhook.dll"
  File "vs_vista_64\readme.txt"
 
  StrCmp $OptConfigs 0 SkipVista64_1
  DetailPrint "Vista 64 bit Optional Configs"

  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
  File "vs_vista_64\blackbox.rc"
  File "vs_vista_64\bsetroot.rc"
  File "vs_vista_64\extensions.rc"
  File "vs_vista_64\menu.rc"
  File "vs_vista_64\plugins.rc"
  File "vs_vista_64\shellfolders.rc"
  File "vs_vista_64\stickywindows.ini"

SkipVista64_1:

  StrCmp $OptStyles 0 SkipVista64_2
  DetailPrint "Vista 64 bit Optional Styles"
 
  SetOutPath $INSTDIR\backgrounds
  File /r "vs_vista_64\backgrounds\"
  SetOutPath $INSTDIR\styles
  File /r "vs_vista_64\styles\"

SkipVista64_2:

  StrCmp $OptCorePlugins 0 SkipVista64_3
  DetailPrint "Vista 64 bit Optional Core Plugins"

  #SetOutPath $INSTDIR\plugins\bbAnalog
	#File /r "vs_vista_64\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbColor3dc
	File /r "vs_vista_64\plugins\bbColor3dc\"
  SetOutPath $INSTDIR\plugins\bbIconBox
	File /r "vs_vista_64\plugins\bbIconBox\"
  #SetOutPath $INSTDIR\plugins\bbInterface
	#File /r "vs_vista_64\plugins\bbInterface\"
  SetOutPath $INSTDIR\plugins\bbKeys
	File /r "vs_vista_64\plugins\bbKeys\"
  SetOutPath $INSTDIR\plugins\BlackboxBar
	File /r "vs_vista_64\plugins\BlackboxBar\"
  SetOutPath $INSTDIR\plugins\bbLeanSkin
	File /r "vs_vista_64\plugins\bbLeanSkin\"
  SetOutPath $INSTDIR\plugins\bbSlit
	File /r "vs_vista_64\plugins\bbSlit\"

SkipVista64_3:

  StrCmp $OptPlugins 0 SkipVista64_4
  DetailPrint "Vista 64 bit Optional Plugins 1"

  SetOutPath $INSTDIR\plugins\bbAnalogExMod
	File /r "vs_vista_64\plugins\bbAnalogExMod\"
  SetOutPath $INSTDIR\plugins\bbAnalog
	File /r "vs_vista_64\plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbCalendar
	File /r "vs_vista_64\plugins\bbCalendar\"
  SetOutPath $INSTDIR\plugins\BBDigitalEx
	File /r "vs_vista_64\plugins\BBDigitalEx\"
  SetOutPath $INSTDIR\plugins\bbFooMan
	File /r "vs_vista_64\plugins\bbFooMan\"
  SetOutPath $INSTDIR\plugins\bbMuse
	File /r "vs_vista_64\plugins\bbMuse\"
  SetOutPath $INSTDIR\plugins\bbPlayer
	File /r "vs_vista_64\plugins\bbPlayer\"
  SetOutPath $INSTDIR\plugins\bbSeekBar
	File /r "vs_vista_64\plugins\bbSeekBar\"
  SetOutPath $INSTDIR\plugins\bbFoomp
	File /r "vs_vista_64\plugins\bbFoomp\"
  SetOutPath $INSTDIR\plugins\BBMagnify
	File /r "vs_vista_64\plugins\BBMagnify\"
  SetOutPath $INSTDIR\plugins\bbPager
	File /r "vs_vista_64\plugins\bbPager\"
  SetOutPath $INSTDIR\plugins\bbRecycleBin
	File /r "vs_vista_64\plugins\bbRecycleBin\"
  SetOutPath $INSTDIR\plugins\BBRSS
	File /r "vs_vista_64\plugins\BBRSS\"
  SetOutPath $INSTDIR\plugins\BBStyle
	File /r "vs_vista_64\plugins\BBStyle\"
  SetOutPath $INSTDIR\plugins\BBSysMeter
	File /r "vs_vista_64\plugins\BBSysMeter\"
  SetOutPath $INSTDIR\plugins\bbWorkspaceWheel
	File /r "vs_vista_64\plugins\bbWorkspaceWheel\"
  SetOutPath $INSTDIR\plugins\SystemBarEx
	File /r "vs_vista_64\plugins\SystemBarEx\"

SkipVista64_4:

  StrCmp $OptPlugins2 0 SkipVista64_5
  DetailPrint "Vista 64 bit Optional Plugins 2"

  SetOutPath $INSTDIR\plugins\BB8Ball
	File /r "vs_vista_64\plugins\BB8Ball\"
  SetOutPath $INSTDIR\plugins\bbInterface_iTunes
	File /r "vs_vista_64\plugins\bbInterface_iTunes\"
  SetOutPath $INSTDIR\plugins\BBMessageBox
	File /r "vs_vista_64\plugins\BBMessageBox\"
  SetOutPath $INSTDIR\plugins\BBXO
	File /r "vs_vista_64\plugins\BBXO\"

SkipVista64_5:

SectionEnd


# windows detection
Function windetectionPageEnter
  ${If} ${AtLeastWinVista}
    ${If} ${RunningX64}
        StrCpy $win_xp 0
        StrCpy $win_64 1
    ${Else}
        StrCpy $win_xp 0
        StrCpy $win_64 0
    ${EndIf}
  ${Else}
    ${If} ${RunningX64}
        StrCpy $win_xp 1
        StrCpy $win_64 1
    ${Else}
        StrCpy $win_xp 1
        StrCpy $win_64 0
    ${EndIf}
  ${EndIf} 

  nsDialogs::Create 1018
  Pop $BuildVerDialog

  ${NSD_CreateGroupBox} 2% 2% 48% 98% "Windows"
  Pop $GroupBox1

  ${NSD_CreateRadioButton} 5% 33% 40% 6% "XP"
    Pop $Group1BuildVerRadioXP
    ${NSD_AddStyle} $Group1BuildVerRadioXP ${WS_GROUP}
  ${NSD_CreateRadioButton} 5% 66% 40% 6% "Vista, Win7 or Win8"
    Pop $Group1BuildVerRadioVista

  ${NSD_CreateGroupBox} 52% 2% 46% 98% "bits"
  Pop $GroupBox2
 
  ${NSD_CreateRadioButton} 55% 33% 40% 6% "32"
    Pop $Group1BuildVer2Radio32
    ${NSD_AddStyle} $Group1BuildVer2Radio32 ${WS_GROUP}
  ${NSD_CreateRadioButton} 55% 66% 40% 6% "64"
    Pop $Group1BuildVer2Radio64

  ${If} $win_xp == 1
    ${NSD_SetState} $Group1BuildVerRadioXP ${BST_CHECKED}
  ${Else}
    ${NSD_SetState} $Group1BuildVerRadioVista ${BST_CHECKED}
  ${EndIf}

  ${If} $win_64 == 1
    ${NSD_SetState} $Group1BuildVer2Radio64 ${BST_CHECKED}
  ${Else}
    ${NSD_SetState} $Group1BuildVer2Radio32 ${BST_CHECKED}
  ${EndIf}

  nsDialogs::Show
FunctionEnd
 
Var radio_xp
Var radio_64

Function windetectionPageLeave
  ${NSD_GetState} $Group1BuildVerRadioXP $radio_xp
  ${NSD_GetState} $Group1BuildVer2Radio64 $radio_64

  ${If} $radio_xp == ${BST_CHECKED}
    ${If} $radio_64 == ${BST_CHECKED}
        SectionSetFlags ${Sec_XP_64} ${SF_SELECTED}
    ${Else}
        SectionSetFlags ${Sec_XP_32} ${SF_SELECTED}
    ${EndIf}
  ${Else}
    ${If} $radio_64 == ${BST_CHECKED}
        SectionSetFlags ${Sec_Vista_64} ${SF_SELECTED}
    ${Else}
        SectionSetFlags ${Sec_Vista_32} ${SF_SELECTED}
    ${EndIf}
  ${EndIf}

  call redistPageEnter
FunctionEnd

### msvc redist
Function redistPageEnter

  File redist\vcredist_x64.exe
  File redist\vcredist_x86.exe

  ${If} $radio_64 == 1
    ExecWait 'vcredist_x64.exe /install /passive'
    ExecWait 'vcredist_x86.exe /install /passive'
  ${Else}
    ExecWait 'vcredist_x86.exe /install /passive'
  ${EndIf}
FunctionEnd

### as shell dialogue
Function shellPageEnter
  nsDialogs::Create 1018
  Pop $ShellDialog

  ${NSD_CreateGroupBox} 2% 2% 98% 48% "Choose how to install BlackBox:"
  Pop $GroupBox3

  ${NSD_CreateRadioButton} 10% 18% 95% 6% "no, do NOT install as shell (default)"
    Pop $RadioButtonNoShell
    ${NSD_AddStyle} $RadioButtonNoShell ${WS_GROUP}
  ${NSD_CreateRadioButton} 10% 32% 95% 6% "yes, install BlackBox as shell and replace explorer.exe"
    Pop $RadioButtonAsShell

  ${NSD_SetState} $RadioButtonNoShell ${BST_CHECKED}
  ${NSD_SetState} $RadioButtonAsShell ${BST_UNCHECKED}

  ${NSD_CreateGroupBox} 2% 52% 98% 98% "Choose affected user:"
  Pop $GroupBox4

  ${NSD_CreateRadioButton} 10% 68% 95% 6% "install BlackBox for current user (default)"
    Pop $RadioButtonCurrUser
    ${NSD_AddStyle} $RadioButtonCurrUser ${WS_GROUP}
  ${NSD_CreateRadioButton} 10% 84% 95% 6% "install BlackBox for all users (requires admin rights)"
    Pop $RadioButtonAllUser

  ${NSD_SetState} $RadioButtonCurrUser ${BST_CHECKED}
  ${NSD_SetState} $RadioButtonAllUser ${BST_UNCHECKED}

  nsDialogs::Show
FunctionEnd
 
Function shellPageLeave
  ${NSD_GetState} $RadioButtonAsShell $as_shell
  ${NSD_GetState} $RadioButtonCurrUser $curr_user
FunctionEnd

Function .onInit
FunctionEnd

Function .OnInstSuccess
  ${If} $as_shell == ${BST_CHECKED}
    ${If} $curr_user == ${BST_CHECKED}
      ExecWait "$INSTDIR\bsetshell.exe -b -u"
    ${Else}
      ExecWait "$INSTDIR\bsetshell.exe -b"
    ${EndIf}
    MessageBox mb_iconstop "BlackBox has been set as shell. Logout to apply."
  ${Else}
  ${EndIf}
FunctionEnd

#Function .OnInstFailed
#    UAC::Unload ;Must call unload!
#FunctionEnd



