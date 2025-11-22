; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

#define MyAppName "MagicBox"
#define MyAppVersion "1.0.0"
#define MyAppExeName "magic_box.exe"
#define SExeName "MagicBoxSearchService.exe"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (若要生成新的 GUID，可在菜单中点击 "工具|生成 GUID"。)
AppId={{79552450-AE33-45EA-B2C3-D5671D6E07A3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; 以下行取消注释，以在非管理安装模式下运行（仅为当前用户安装）。
;PrivilegesRequired=lowest
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline
OutputDir=J:\zzx\Code\Flutter\magic_box\release
OutputBaseFilename=MagicBoxSetup
SetupIconFile=J:\zzx\Code\Flutter\magic_box\windows\runner\resources\app_icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "J:\zzx\Code\Flutter\magic_box\build\windows\x64\runner\Release\magic_box.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "J:\zzx\Code\Flutter\magic_box\build\windows\x64\runner\Release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "inplug\*"
Source: "J:\zzx\Code\Flutter\magic_box\build\windows\x64\runner\Release\inplug\*"; DestDir: "{app}\inplug_temp"; Flags: ignoreversion recursesubdirs createallsubdirs
; 注意: 不要在任何共享系统文件上使用"Flags: ignoreversion"

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\{#SExeName}"; Description: "快捷键一键唤醒服务"; Flags: nowait skipifsilent

[Code]
procedure CopyFileWithProgress(const SourceFile, DestFile: string);
begin
  if FileCopy(SourceFile, DestFile, False) then
    DeleteFile(SourceFile);
end;

procedure CopyDirectoryContents(const SourceDir, DestDir: string);
var
  FindRec: TFindRec;
  SourceFile, DestFile: string;
begin
  if FindFirst(SourceDir + '\*', FindRec) then
  begin
    try
      repeat
        if (FindRec.Name <> '.') and (FindRec.Name <> '..') then
        begin
          SourceFile := SourceDir + '\' + FindRec.Name;
          DestFile := DestDir + '\' + FindRec.Name;
          
          if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0 then
          begin
            // 是目录，递归复制
            if not DirExists(DestFile) then
              ForceDirectories(DestFile);
            CopyDirectoryContents(SourceFile, DestFile);
            // 复制完成后删除源目录
            DelTree(SourceFile, True, True, True);
          end
          else
          begin
            // 是文件，直接复制
            if FileCopy(SourceFile, DestFile, False) then
              DeleteFile(SourceFile);
          end;
        end;
      until not FindNext(FindRec);
    finally
      FindClose(FindRec);
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  AppDataPath, SourceDir: string;
begin
  if CurStep = ssPostInstall then
  begin
    // 获取AppData路径
    AppDataPath := ExpandConstant('{userappdata}\com.dmqj\magic_box');
    
    // 创建目标目录
    if not DirExists(AppDataPath) then
      ForceDirectories(AppDataPath);
    
    // 移动inplug_temp中的内容到AppData目录
    SourceDir := ExpandConstant('{app}\inplug_temp');
    if DirExists(SourceDir) then
    begin
      // 复制目录内容
      CopyDirectoryContents(SourceDir, AppDataPath);
      
      // 删除临时目录
      DelTree(SourceDir, True, True, True);
    end;
  end;
end;

// 结束指定名称的进程
procedure KillProcess(const ProcessName: string);
var
  ResultCode: Integer;
begin
  // 使用taskkill命令结束进程，/F表示强制结束，/IM表示按映像名称指定进程
  Exec('taskkill.exe', '/F /IM ' + ProcessName, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

// 卸载步骤变更时触发
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // 在卸载开始前结束相关进程
  if CurUninstallStep = usUninstall then
  begin
    // 结束搜索服务进程
    KillProcess('{#SExeName}');
    // 结束主程序进程
    KillProcess('{#MyAppExeName}');
  end;
end;