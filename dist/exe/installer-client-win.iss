; Inno install packager - http://www.jrsoftware.org/isinfo.php

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{22222C83-6BF9-4372-9226-73768939CB3D}}
AppName=MAngband
#define VERSION "1.5.3"
AppVersion={#VERSION}
AppPublisher=MAngband Project Team
AppPublisherURL=https://mangband.org
AppSupportURL=
AppUpdatesURL=
DefaultDirName={localappdata}\MAngband1.5
DefaultGroupName=MAngband
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=mangband-client-setup-v{#VERSION}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\mangclient.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangclient-sdl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangclient-sdl2.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\sdl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\sdl_ttf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\sdl2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\sdl2_ttf.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\libfreetype-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangclient.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "README-client.htm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\lib\bone\*"; DestDir: "{app}\lib\bone"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\pref\*"; DestDir: "{app}\lib\pref"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\user\*"; DestDir: "{app}\lib\user"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\xtra\font\*"; DestDir: "{app}\lib\xtra\font"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\xtra\graf\*"; DestDir: "{app}\lib\xtra\graf"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\xtra\sound\*"; DestDir: "{app}\lib\xtra\sound"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MAngband"; Filename: "{app}\mangclient.exe"
Name: "{group}\MAngband (One Window)"; Filename: "{app}\mangclient-sdl.exe"
Name: "{group}\MAngband (Dockable Windows)"; Filename: "{app}\mangclient-sdl2.exe"
Name: "{group}\ReadMe"; Filename: "{app}\README-client.htm"
Name: "{group}\Config Dir"; Filename: "{app}\lib\user\"
Name: "{group}\{cm:UninstallProgram,MAngband}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\MAngband"; Filename: "{app}\mangclient.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\mangclient.exe"; Description: "{cm:LaunchProgram,MAngband}"; Flags: nowait postinstall skipifsilent
