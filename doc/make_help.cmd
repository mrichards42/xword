python markdown.py
python link_check.py
"%programfiles(x86)%\HTML Help Workshop\hhc" chm\help.hhp
:: hhc exits with errorlevel = 1 on success - don't treat this as a failure.
if %errorlevel% equ 1 exit /B 0 else exit /B 1
