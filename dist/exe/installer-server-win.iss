; Inno install packager - http://www.jrsoftware.org/isinfo.php

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{C44CA705-B1D7-4A64-91A7-2AB339E10BBE}}
AppName=MAngband Server
#define VERSION "1.5.3"
AppVersion={#VERSION}
AppPublisher=MAngband Project Team
AppPublisherURL=https://mangband.org
AppSupportURL=
AppUpdatesURL=
DefaultDirName={localappdata}\MAngbandServer1.5
DefaultGroupName=MAngbandServer
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=mangband-server-setup-v{#VERSION}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]

[Files]
Source: "..\..\mangband.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangband.cfg"; DestDir: "{app}"; Flags: ignoreversion
Source: "run-server.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.htm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\lib\bone\*"; DestDir: "{app}\lib\bone"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\data\*"; DestDir: "{app}\lib\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\edit\*"; DestDir: "{app}\lib\edit"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\help\*"; DestDir: "{app}\lib\help"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\pref\*"; DestDir: "{app}\lib\pref"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\save\*"; DestDir: "{app}\lib\save"; Flags: ignoreversion recursesubdirs createallsubdirs


[Icons]
Name: "{group}\MAngbandServer"; Filename: "{app}\run-server.bat"
Name: "{group}\ReadMe"; Filename: "{app}\README.htm"
Name: "{group}\Config Dir"; Filename: "{app}\"
Name: "{group}\{cm:UninstallProgram,MAngband Server}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\run-server.bat"; Description: "{cm:LaunchProgram,MAngband Server}"; Flags: nowait postinstall skipifsilent
