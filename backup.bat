SET NEWTIME=%TIME::=%
SET NEWTIME=%NEWTIME:~0,6%

SET PPJOYDIR=%~dp0
CD /D "%PPJOYDIR%"

SET BACKUPNAME="__Backups__\ppJoyBackup_%DATE:/=%_%NEWTIME%"
zip -9 -X -r %BACKUPNAME% Docs\* "Visio Diagrams\*" IOCTLSample\* Web\*
zip -9 -X -r %BACKUPNAME% Branding\* CreateBAT\* CreateINF\*
zip -9 -X %BACKUPNAME% AddJoyDrivers\* ViseHelper\* SetupHelper\* Helper64\* UnInst64\*
zip -9 -X %BACKUPNAME% PPJoyBus\* PPJoyInc\* PPortJoy\* PPJoyCpl\* W98Ports\* 
zip -9 -X %BACKUPNAME% PPJoyKey\* PPJoyCom\* PPJoyDLL\* PPJoyMouse\* RCCallbackDLLs\* PPJoyJoy\*
zip -9 -X %BACKUPNAME% PPJoyAPI\* PPJoyAPI_turboC++\* PPJoyAPI_turboC++\Release_Build\PPJoyAPI.lib
zip -9 -X %BACKUPNAME% Tools\AddDriversTest\* Tools\APITest\* Tools\ColCrop\* Tools\DecodeAeroChopper\* Tools\GenerateSerialStream\*
zip -9 -X %BACKUPNAME% Tools\MakeBitSwapLookup\* Tools\PPJoyCOMLogToBin\* Tools\PPJoyDLL_TestDLL\* Tools\SetVirtualJoyState\*
zip -9 -X %BACKUPNAME% Installer\* Installer\pluginapi\* TestSign\*
zip -9 -X -r %BACKUPNAME% *.bat *.txt dirs 

rem temp
zip -9 -X %BACKUPNAME% PPJoyCpl_old_ANSI\*

rem old stuff...
rem zip -9 -X %BACKUPNAME% Installer\*.VCT Web\WebSpecific\*


Pause