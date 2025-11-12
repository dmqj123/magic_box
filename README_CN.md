# Magic Box

一款使用 Flutter 构建的强大且多功能的桌面搜索工具。Magic Box 是一个浮动搜索工具，允许用户使用各种插件快速搜索和检索信息，提供从单一界面访问文件、应用程序和网络内容的高效方式。

## 功能特性

- **浮动窗口**: 透明、始终置顶的搜索窗口，在工作时保持可访问
- **插件系统**: 支持自定义插件的可扩展架构，用于不同的搜索功能
- **实时搜索**: 键入时即时搜索结果
- **跨平台**: 使用 Flutter 构建，支持 Windows、macOS 和 Linux
- **可自定义界面**: 设置面板和插件管理
- **键盘控制**: 支持键盘快捷键和导航

## 架构

Magic Box 设计为基于插件的搜索工具：
- **主应用程序**: 提供搜索界面的 Flutter UI
- **插件系统**: 执行特定搜索任务的外部可执行文件
- **通信**: 插件通过标准输入/输出的 JSON 与主应用通信
- **插件格式**: 插件以包含可执行文件和配置的 ZIP 包形式分发

## 使用方法

1. 启动 Magic Box - 它显示为一个小的浮动搜索栏
2. 在搜索框中开始输入以开始搜索
3. 启用的插件结果将显示在搜索栏下方
4. 单击结果以执行其关联操作
5. 通过齿轮图标访问设置以管理插件和首选项
6. 使用关闭 (X) 按钮关闭 Magic Box 或按 Escape 键
7. 使用Ctrl+~键在任意地方打开 Magic Box 进行快速搜索

## 插件开发

Magic Box 支持插件系统，允许开发人员扩展其功能：

### 插件结构
```
plugin_name.zip
├── config.json
├── plugin_executable.exe
├── icon.png (可选)
└── other_files...
```

### Config.json 格式
```json
{
  "sign": "mbep",
  "name": "插件名称",
  "version": "1.0.0",
  "main_file": "plugin_executable.exe",
  "icon": "icon.png" (可选)
}
```

### 插件通信协议
- 插件通过命令行参数（使用 -k 标志）接收搜索查询
- 插件通过标准输出以 JSON 格式返回结果
- 多个结果可以用 `next_result` 分隔符分隔

### JSON 结果格式
```json
{
  "title": "结果标题",
  "content": "结果内容/描述",
  "cmd": "单击时执行的命令",
  "preview_path": "预览图像路径 (可选)",
  "encoding": "文本编码 (可选)",
  "auto_close": false //可选 打开结果后是否自动关闭窗口
}
```

## 设置

Magic Box 包含一个设置界面用于：
- 管理已安装的插件
- 配置全局热键 (即将推出)
- 调整外观和行为

## 支持

如果您遇到任何问题或对 Magic Box 有疑问：

1. 查看 [Issues](https://github.com/dmqj123/magic_box/issues) 部分
2. 如果您的问题尚未解决，报告新问题
3. 请随时通过 fork 和提交 pull request 来为项目做贡献