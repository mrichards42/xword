; NB: This must be compiled with Unicode NSIS so that the extra long strings
; in the config section don't get truncated.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "XWord"
!define PRODUCT_VERSION "0.5.3"
!define PRODUCT_WEB_SITE "http://sourceforge.net/projects/wx-xword/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\XWord.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define XWORD_TRUNK "D:\C++\XWord\trunk"
!define XWORD_BIN "${XWORD_TRUNK}\bin\Release"
!define VC_REDIST "vc_redist"

SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; -----------------------------------------------------------------------------
; Pages
; -----------------------------------------------------------------------------

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!insertmacro MUI_PAGE_LICENSE "${XWORD_TRUNK}\dist\gpl-3.0.rtf"


; Portable page (custom)
LangString PORTABLE_PAGE_TITLE ${LANG_ENGLISH} "Choose Install Type"
LangString PORTABLE_PAGE_SUBTITLE ${LANG_ENGLISH} "Choose standard installation or portable installation."

!include "nsDialogs.nsh"
!include "winmessages.nsh"
!include "logiclib.nsh"

Page Custom PortablePre PortablePost

var dialog
var hwnd
var isPortable
var isInstallDirSet
var CONFIGDIR
var SCRIPTSDIR

Function PortablePre
    ; Default value for $isPortable
    ${If} $isPortable == ""
        StrCpy $isPortable "false"
    ${EndIf}

    !insertmacro MUI_HEADER_TEXT $(PORTABLE_PAGE_TITLE) $(PORTABLE_PAGE_SUBTITLE)

	nsDialogs::Create 1018
		Pop $dialog

    ; Radio buttons
	${NSD_CreateRadioButton} 0 0 40% 6% "Standard Installation"
		Pop $hwnd
		${NSD_AddStyle} $hwnd ${WS_GROUP}
		nsDialogs::SetUserData $hwnd "false"
		${NSD_OnClick} $hwnd RadioClick
                ; Is this initially selected?
                ${If} $isPortable == "false"
                    ${NSD_Check} $hwnd
                ${EndIf}
	${NSD_CreateRadioButton} 0 12% 40% 6% "Portable Installation"
		Pop $hwnd
		nsDialogs::SetUserData $hwnd "true"
		${NSD_OnClick} $hwnd RadioClick
                ; Is this initially selected?
                ${If} $isPortable == "true"
                      ${NSD_Check} $hwnd
                ${EndIf}

	nsDialogs::Show
FunctionEnd

; Callback for radio buttons
Function RadioClick
	Pop $hwnd
	nsDialogs::GetUserData $hwnd
	Pop $isPortable
FunctionEnd

Function PortablePost
    ; Set the default install dir based based on the selection.
    ; Only do this the first time we visit the page
    ${If} $isInstallDirSet == ""
        ${If} $isPortable == "true"
            StrCpy $INSTDIR "$DESKTOP\XWord"
        ${Else}
            StrCpy $INSTDIR "$PROGRAMFILES\XWord"
        ${EndIf}
        StrCpy $isInstallDirSet "true"
    ${EndIf}
FunctionEnd



; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "XWord"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!define MUI_PAGE_CUSTOMFUNCTION_PRE startMenuPagePre
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP
Function startMenuPagePre
         ; No need for start menu icons if this is a portable installation
         ${If} $isPortable == "true"
               Abort
         ${EndIf}
FunctionEnd

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\XWord.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\XWord" ; This will be overwritten by the portable page
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

; Files
Section "MainSection" SEC01
    ; Set data folders (scripts, config files)
    ${If} $isPortable == "true"
        StrCpy $CONFIGDIR "$INSTDIR\config"
    ${Else}
        StrCpy $CONFIGDIR "$APPDATA\XWord\config"
    ${EndIf}
    StrCpy $SCRIPTSDIR "$INSTDIR\scripts"

    SetOutPath "$INSTDIR"
        ; Program
        File "${XWORD_BIN}\XWord.exe"
        File "${XWORD_BIN}\puz.dll"
        ; LUA
        File "${XWORD_BIN}\lua5.1.dll"
        File "${XWORD_BIN}\luapuz.dll"
        ; Parsers / Misc
        File "${XWORD_BIN}\libexpat.dll"
        File "${XWORD_BIN}\yajl.dll"
        File "${XWORD_BIN}\zlib1.dll"
        File "${XWORD_BIN}\libcurl.dll"
        ; Info
        File "${XWORD_TRUNK}\README"
        File "${XWORD_TRUNK}\AUTHORS"
        File "${XWORD_TRUNK}\COPYING"
        File "${XWORD_TRUNK}\CHANGELOG"

    ; VC Redist
    SetOutPath "$INSTDIR\Microsoft.VC90.CRT"
        File "${VC_REDIST}\Microsoft.VC90.CRT\msvcm90.dll"
        File "${VC_REDIST}\Microsoft.VC90.CRT\msvcp90.dll"
        File "${VC_REDIST}\Microsoft.VC90.CRT\msvcr90.dll"
        File "${VC_REDIST}\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest"

    SetOutPath "$INSTDIR\images"
        File "${XWORD_TRUNK}\images\delete_24.png"
        File "${XWORD_TRUNK}\images\check_grid_24.png"
        File "${XWORD_TRUNK}\images\check_letter_24.png"
        File "${XWORD_TRUNK}\images\check_word_24.png"
        File "${XWORD_TRUNK}\images\layout_24.png"
        File "${XWORD_TRUNK}\images\notes_24.png"
        File "${XWORD_TRUNK}\images\notes_new_24.png"
        File "${XWORD_TRUNK}\images\open_24.png"
        File "${XWORD_TRUNK}\images\save_24.png"
        File "${XWORD_TRUNK}\images\timer_24.png"
        File "${XWORD_TRUNK}\images\zoom_fit_24.png"
        File "${XWORD_TRUNK}\images\zoom_in_24.png"
        File "${XWORD_TRUNK}\images\zoom_out_24.png"

    SetOutPath "$CONFIGDIR"
        ; Default layouts
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Full\ View" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=515;besth=377;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=258;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=105;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=140;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=515;besth=73;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=246;besth=494;minw=15;minh=15;maxw=-1;maxh=-1;floatx=280;floaty=194;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=4;layer=4;row=0;pos=0;prop=100000;bestw=246;besth=494;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-12;floaty=517;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|dock_size(4,4,0)=238|dock_size(4,5,0)=238|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Floating\ Clues" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=391;besth=377;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=195;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=79;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=105;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=391;besth=73;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875389;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=236;besth=494;minw=15;minh=15;maxw=-1;maxh=-1;floatx=205;floaty=83;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875389;dir=4;layer=4;row=0;pos=0;prop=100000;bestw=236;besth=494;minw=15;minh=15;maxw=-1;maxh=-1;floatx=349;floaty=424;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Side\ Clues" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=382;besth=360;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=190;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=77;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=103;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=382;besth=56;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=236;besth=477;minw=15;minh=15;maxw=-1;maxh=-1;floatx=280;floaty=194;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=2;layer=4;row=0;pos=0;prop=100000;bestw=236;besth=477;minw=15;minh=15;maxw=-1;maxh=-1;floatx=431;floaty=262;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|dock_size(4,5,0)=238|dock_size(2,4,0)=238|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Stacked\ Clues\ \(Left\)" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=624;besth=377;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=314;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=128;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=170;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=624;besth=73;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=236;besth=244;minw=15;minh=15;maxw=-1;maxh=-1;floatx=280;floaty=194;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=4;layer=5;row=0;pos=1;prop=100000;bestw=236;besth=244;minw=15;minh=15;maxw=-1;maxh=-1;floatx=1165;floaty=254;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|dock_size(4,5,0)=284|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Stacked\ Clues\ \(Right\)" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=425;besth=360;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=212;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=86;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=115;besth=15;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=425;besth=56;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=2;layer=4;row=0;pos=0;prop=100000;bestw=241;besth=227;minw=15;minh=15;maxw=-1;maxh=-1;floatx=561;floaty=246;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=2;layer=4;row=0;pos=1;prop=100000;bestw=241;besth=227;minw=15;minh=15;maxw=-1;maxh=-1;floatx=553;floaty=248;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|dock_size(2,4,0)=243|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Clues\ Below" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=672;besth=337;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=339;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=138;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=183;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=672;besth=73;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=3;layer=0;row=1;pos=0;prop=100000;bestw=333;besth=34;minw=15;minh=15;maxw=-1;maxh=-1;floatx=1134;floaty=300;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=3;layer=0;row=1;pos=1;prop=100000;bestw=333;besth=34;minw=15;minh=15;maxw=-1;maxh=-1;floatx=1158;floaty=296;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=75|dock_size(3,0,1)=28|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "Clues\ Above" "layout2|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=672;besth=346;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=153658;bestw=339;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=63114;bestw=138;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=183;besth=32;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=1;pos=0;prop=100000;bestw=672;besth=73;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=365;besth=288;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=365;floath=288|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=333;besth=25;minw=15;minh=15;maxw=-1;maxh=-1;floatx=724;floaty=709;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=1;layer=2;row=0;pos=1;prop=100000;bestw=333;besth=25;minw=15;minh=15;maxw=-1;maxh=-1;floatx=730;floaty=718;floatw=300;floath=323|dock_size(5,0,0)=22|dock_size(1,3,0)=34|dock_size(1,2,0)=30|dock_size(1,2,1)=75|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "\(Previous\)" "layout2|name=Logger;caption=Logger;state=18876414;dir=4;layer=6;row=0;pos=0;prop=100000;bestw=735;besth=359;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=4;floaty=78;floatw=735;floath=359|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=718;besth=691;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=149558;bestw=352;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=67214;bestw=157;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=197;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=718;besth=31;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=485;besth=264;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=485;floath=264|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|name=ClueList1;caption=Across;state=18875388;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=339;besth=744;minw=15;minh=15;maxw=-1;maxh=-1;floatx=280;floaty=194;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=4;layer=4;row=0;pos=0;prop=100000;bestw=339;besth=744;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-12;floaty=517;floatw=300;floath=323|dock_size(5,0,0)=43|dock_size(1,3,0)=28|dock_size(1,2,0)=33|dock_size(4,1000,0)=24|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=Logger;|frame_size=1440,762|"
        WriteINIStr "$CONFIGDIR\config.ini" "Layouts" "\(Current\)" "layout2|name=Logger;caption=Logger;state=18876414;dir=4;layer=6;row=0;pos=0;prop=100000;bestw=735;besth=359;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=4;floaty=78;floatw=735;floath=359|name=Grid;caption=Grid;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=708;besth=675;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=647|name=Title;caption=Title;state=18875388;dir=1;layer=3;row=0;pos=0;prop=149558;bestw=347;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=86;floath=20|name=Author;caption=Author;state=18875388;dir=1;layer=3;row=0;pos=1;prop=67214;bestw=155;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Copyright;caption=Copyright;state=18875388;dir=1;layer=3;row=0;pos=2;prop=83228;bestw=194;besth=26;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=87;floath=20|name=Clue;caption=Clue Prompt;state=18875388;dir=1;layer=2;row=0;pos=0;prop=100000;bestw=708;besth=31;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=272;floath=75|name=Notes;caption=Notes;state=18876415;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=485;besth=264;minw=15;minh=15;maxw=-1;maxh=-1;floatx=331;floaty=206;floatw=485;floath=264|name=ClueList1;caption=Across;state=18875388;dir=4;layer=5;row=0;pos=0;prop=100000;bestw=339;besth=744;minw=15;minh=15;maxw=-1;maxh=-1;floatx=280;floaty=194;floatw=300;floath=322|name=ClueList2;caption=Down;state=18875388;dir=4;layer=4;row=0;pos=0;prop=100000;bestw=339;besth=744;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-12;floaty=517;floatw=300;floath=323|name=00520f304e5da8570000014700000007;caption=Notes;state=18876414;dir=4;layer=0;row=0;pos=1;prop=100000;bestw=250;besth=250;minw=15;minh=15;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=250;floath=250|dock_size(5,0,0)=43|dock_size(1,3,0)=28|dock_size(1,2,0)=33|dock_size(4,1000,0)=24|dock_size(4,5,0)=344|dock_size(4,4,0)=344|__tab(1)=|__tab(2)=|__tab(3)=|__tab(4)=Logger;|frame_size=1440,762|"

    SetOutPath "$SCRIPTSDIR"

    ; Script libraries

    SetOutPath "$SCRIPTSDIR\libs"
        ; Anywhere we have dlls that link to msvcrt, we need a manifest file
        File "${VC_REDIST}\dummy_manifest\Microsoft.VC90.CRT.manifest"

        File "${XWORD_TRUNK}\scripts\libs\date.lua"
        File "${XWORD_TRUNK}\scripts\libs\lfs.dll"
        File "${XWORD_TRUNK}\scripts\libs\lsqlite3.dll"
        File "${XWORD_TRUNK}\scripts\libs\luacurl.dll"
        File "${XWORD_TRUNK}\scripts\libs\lxp.dll"
        File "${XWORD_TRUNK}\scripts\libs\safe_exec.lua"
        File "${XWORD_TRUNK}\scripts\libs\serialize.lua"
        File "${XWORD_TRUNK}\scripts\libs\task.dll"
    SetOutPath "$SCRIPTSDIR\libs\lxp"
        File "${XWORD_TRUNK}\scripts\libs\lxp\lom.lua"
    SetOutPath "$SCRIPTSDIR\libs\pl"
        File "${XWORD_TRUNK}\scripts\libs\pl\array2d.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\class.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\classx.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\comprehension.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\config.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\data.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\dir.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\func.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\init.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\input.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\lapp.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\lexer.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\list.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\luabalanced.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\operator.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\path.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\permute.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\pretty.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\seq.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\sip.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\strict.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\stringio.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\stringx.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\tablex.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\test.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\text.lua"
        File "${XWORD_TRUNK}\scripts\libs\pl\utils.lua"
    SetOutPath "$SCRIPTSDIR\libs\wxtask"
        File "${XWORD_TRUNK}\scripts\libs\wxtask\init.lua"
        File "${XWORD_TRUNK}\scripts\libs\wxtask\queue.lua"

    ; XWord main script
    SetOutPath "$SCRIPTSDIR\xword"
        File "${XWORD_TRUNK}\scripts\xword\cleanup.lua"
        File "${XWORD_TRUNK}\scripts\xword\init.lua"
        File "${XWORD_TRUNK}\scripts\xword\menu.lua"
        File "${XWORD_TRUNK}\scripts\xword\messages.lua"
    SetOutPath "$SCRIPTSDIR\xword\pkgmgr"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\dialog.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\init.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\install.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\join.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\load.lua"
    SetOutPath "$SCRIPTSDIR\xword\pkgmgr\updater"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\updater\check_task.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\updater\dialog.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\updater\download_task.lua"
        File "${XWORD_TRUNK}\scripts\xword\pkgmgr\updater\init.lua"
    SetOutPath "$SCRIPTSDIR\xword\widgets"
        File "${XWORD_TRUNK}\scripts\xword\widgets\arrows.png"
        File "${XWORD_TRUNK}\scripts\xword\widgets\button.lua"
        File "${XWORD_TRUNK}\scripts\xword\widgets\customlist.lua"
        File "${XWORD_TRUNK}\scripts\xword\widgets\sizedtext.lua"

    ; import script
    SetOutPath "$SCRIPTSDIR\import"
        File "${XWORD_TRUNK}\scripts\import\info.lua"
        File "${XWORD_TRUNK}\scripts\import\init.lua"
        File "${XWORD_TRUNK}\scripts\import\uclick.lua"

    ; If it's portable, make a portable_mode_enabled file.
    ${If} $isPortable == "true"
        FileOpen $0 "$INSTDIR\portable_mode_enabled" w
        FileClose $0
    ${EndIf}

    ; Shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ${If} $isPortable != "true"
        CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
        CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\XWord.lnk" "$INSTDIR\XWord.exe"
    ${EndIf}
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -AdditionalIcons
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ${If} $isPortable != "true"
        CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\uninst.exe"
    ${EndIf}
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section -Post
    ${If} $isPortable != "true"
        WriteUninstaller "$INSTDIR\uninst.exe"
        WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\XWord.exe"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\XWord.exe"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"

        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "ConfigDir" "$CONFIGDIR"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstDir" "$INSTDIR"
        WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "ScriptsDir" "$SCRIPTSDIR"
    ${EndIf}
SectionEnd


Function un.onUninstSuccess
    HideWindow
    MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
    Abort
FunctionEnd

Section Uninstall
    !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
    ReadRegStr $CONFIGDIR  ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "ConfigDir"
    ReadRegStr $INSTDIR ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstDir"
    ReadRegStr $SCRIPTSDIR ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "ScriptsDir"

    Delete "$INSTDIR\uninst.exe"


    ; For each category of files, check to see if they were installed in
    ; a standard location.  If so, delete the entire folder (recursively)
    ; If they were installed in a non-standard location, just delete the
    ; files that we installed.

    ; Application files
    ${If} $INSTDIR == "$PROGRAMFILES\XWord"
        RMDir /r $INSTDIR
    ${Else}
        Delete "$INSTDIR\images\delete_24.png"
        Delete "$INSTDIR\images\check_grid_24.png"
        Delete "$INSTDIR\images\check_letter_24.png"
        Delete "$INSTDIR\images\check_word_24.png"
        Delete "$INSTDIR\images\layout_24.png"
        Delete "$INSTDIR\images\notes_24.png"
        Delete "$INSTDIR\images\notes_new_24.png"
        Delete "$INSTDIR\images\open_24.png"
        Delete "$INSTDIR\images\save_24.png"
        Delete "$INSTDIR\images\timer_24.png"
        Delete "$INSTDIR\images\zoom_fit_24.png"
        Delete "$INSTDIR\images\zoom_in_24.png"
        Delete "$INSTDIR\images\zoom_out_24.png"
        RMDir  "$INSTDIR\images"

        Delete "$INSTDIR\XWord.exe"
        Delete "$INSTDIR\puz.dll"
        Delete "$INSTDIR\luapuz.dll"
        Delete "$INSTDIR\lua5.1.dll"
        Delete "$INSTDIR\libcurl.dll"
        Delete "$INSTDIR\libexpat.dll"

        Delete "$INSTDIR\README"
        Delete "$INSTDIR\AUTHORS"
        Delete "$INSTDIR\COPYING"
        Delete "$INSTDIR\CHANGELOG"

        ; VC Redist
        Delete "$INSTDIR\Microsoft.VC90.CRT\Microsoft.VC90.CRT.manifest"
        Delete "$INSTDIR\Microsoft.VC90.CRT\msvcm90.dll"
        Delete "$INSTDIR\Microsoft.VC90.CRT\msvcp90.dll"
        Delete "$INSTDIR\Microsoft.VC90.CRT\msvcr90.dll"
        RMDir "$INSTDIR\Microsoft.VC90.CRT"

        RMDir  "$INSTDIR"
    ${EndIf}

    ; Config Files
    RMDir /r $CONFIGDIR

    ; User Data files
    RMDir /r "$APPDATA\XWord"

    ; Scripts
    RMDir /r "$INSTDIR\scripts"

    ; Start menu links
    Delete "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"
    Delete "$SMPROGRAMS\$ICONS_GROUP\XWord.lnk"
    RMDir  "$SMPROGRAMS\$ICONS_GROUP"

    ; Registry
    DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
    DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

    SetAutoClose false
SectionEnd