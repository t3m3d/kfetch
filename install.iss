[Setup]
AppId={{DACEF58F-CFDE-4B3E-A84C-BD08D84D6206}
AppName=kfetch
AppVersion=0.2.1
;AppVerName=KryptonFetch 0.2.1
AppPublisher=KryptonBytes
AppPublisherURL=https://www.kryptonbytes.com/
AppSupportURL=https://www.kryptonbytes.com/
AppUpdatesURL=https://www.kryptonbytes.com/
DefaultDirName=C:\Program Files\KryptonBytes\kfetch\KryptonFetch
UninstallDisplayIcon={app}\kfetch.exe
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
ChangesAssociations=yes
DisableProgramGroupPage=yes
LicenseFile=C:\Users\brian\OneDrive\Documents\GitHub\kfetch\LICENSE
PrivilegesRequiredOverridesAllowed=dialog
OutputBaseFilename=mysetup
SolidCompression=yes
WizardStyle=modern dynamic

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\brian\OneDrive\Documents\GitHub\exe\kfetch.exe"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
Root: HKA; Subkey: "Software\Classes\.myp\OpenWithProgids"; ValueType: string; ValueName: "KryptonFetch.myp"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\KryptonFetch.myp"; ValueType: string; ValueName: ""; ValueData: "KryptonFetch"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\KryptonFetch.myp\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\kfetch.exe,0"
Root: HKA; Subkey: "Software\Classes\KryptonFetch.myp\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\kfetch.exe"" ""%1"""

[Icons]
Name: "{autoprograms}\KryptonFetch"; Filename: "{app}\kfetch.exe"
Name: "{autodesktop}\KryptonFetch"; Filename: "{app}\kfetch.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\kfetch.exe"; Description: "{cm:LaunchProgram,KryptonFetch}"; Flags: nowait postinstall skipifsilent

