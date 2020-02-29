; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "XWord"
!define PRODUCT_VERSION "$%XWORD_VERSION%"
!define PRODUCT_WEB_SITE "http://sourceforge.net/projects/wx-xword/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\XWord.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define XWORD_TRUNK "..\"
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
; -----------------------------------------------------------------------------
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

; End of Portable Page
; -----------------------------------------------------------------------------

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
OutFile "XWord-Windows.exe"
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
        File "${XWORD_BIN}\default_config.ini"
        File "${XWORD_BIN}\puz.dll"
        File "${XWORD_TRUNK}\doc\chm\xword.chm"
        ; LUA
        File "${XWORD_BIN}\lua51.dll"
        File "${XWORD_BIN}\luapuz.dll"
        ; Parsers / Misc
        File "${XWORD_BIN}\libexpat.dll"
        File "${XWORD_BIN}\yajl.dll"
        File "${XWORD_BIN}\yaml.dll"
        File "${XWORD_BIN}\zlib1.dll"
        File "${XWORD_BIN}\libcurl.dll"
        File "${XWORD_BIN}\libeay32.dll"
        File "${XWORD_BIN}\libssl32.dll"
        ; Info
        File "${XWORD_TRUNK}\README.md"
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
        File "${XWORD_TRUNK}\images\check_grid_24.png"
        File "${XWORD_TRUNK}\images\check_letter_24.png"
        File "${XWORD_TRUNK}\images\check_word_24.png"
        File "${XWORD_TRUNK}\images\delete_24.png"
        File "${XWORD_TRUNK}\images\expand_16.png"
        File "${XWORD_TRUNK}\images\help_16.png"
        File "${XWORD_TRUNK}\images\layout_24.png"
        File "${XWORD_TRUNK}\images\notes_24.png"
        File "${XWORD_TRUNK}\images\notes_new_24.png"
        File "${XWORD_TRUNK}\images\open_24.png"
        File "${XWORD_TRUNK}\images\rebus_24.png"
        File "${XWORD_TRUNK}\images\save_24.png"
        File "${XWORD_TRUNK}\images\timer_24.png"
        File "${XWORD_TRUNK}\images\zoom_fit_24.png"
        File "${XWORD_TRUNK}\images\zoom_in_24.png"
        File "${XWORD_TRUNK}\images\zoom_out_24.png"

    ; Scripts - generated by gen_nsis.py
    !include scripts.nsi

    SetOutPath "$SCRIPTSDIR\libs"
        ; Anywhere we have dlls that link to msvcrt, we need a manifest file
        File "${VC_REDIST}\dummy_manifest\Microsoft.VC90.CRT.manifest"

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
        Delete "$INSTDIR\images\check_grid_24.png"
        Delete "$INSTDIR\images\check_letter_24.png"
        Delete "$INSTDIR\images\check_word_24.png"
        Delete "$INSTDIR\images\delete_24.png"
        Delete "$INSTDIR\images\expand_16.png"
        Delete "$INSTDIR\images\help_16.png"
        Delete "$INSTDIR\images\layout_24.png"
        Delete "$INSTDIR\images\notes_24.png"
        Delete "$INSTDIR\images\notes_new_24.png"
        Delete "$INSTDIR\images\open_24.png"
        Delete "$INSTDIR\images\rebus_24.png"
        Delete "$INSTDIR\images\save_24.png"
        Delete "$INSTDIR\images\timer_24.png"
        Delete "$INSTDIR\images\zoom_fit_24.png"
        Delete "$INSTDIR\images\zoom_in_24.png"
        Delete "$INSTDIR\images\zoom_out_24.png"
        RMDir  "$INSTDIR\images"

        Delete "$INSTDIR\XWord.exe"
        Delete "$INSTDIR\xword.chm"
        Delete "$INSTDIR\puz.dll"
        Delete "$INSTDIR\luapuz.dll"
        Delete "$INSTDIR\lua51.dll"
        Delete "$INSTDIR\libcurl.dll"
        Delete "$INSTDIR\libeay32.dll"
        Delete "$INSTDIR\libssl32.dll"
        Delete "$INSTDIR\libexpat.dll"

        Delete "$INSTDIR\README.md"
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
