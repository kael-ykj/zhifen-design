[Setup]
AppName=智分Design
AppVersion=3.1.0
AppPublisher=智分Design
DefaultDirName={autopf}\智分Design
DefaultGroupName=智分Design
OutputDir=Output
OutputBaseFilename=智分Design_V3.1.0_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
DisableProgramGroupPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "附加图标:"; Flags: unchecked

[Files]
Source: "Output\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\智分Design V3.1"; Filename: "{app}\智分Design.exe"
Name: "{group}\卸载智分Design"; Filename: "{uninstallexe}"
Name: "{autodesktop}\智分Design V3.1"; Filename: "{app}\智分Design.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\智分Design.exe"; Description: "运行智分Design V3.1"; Flags: nowait postinstall skipifsilent
