# Magic Box

[简体中文](README_CN.md)

A powerful and versatile desktop search utility built with Flutter. Magic Box is a floating search tool that allows users to quickly search and retrieve information using various plugins, providing an efficient way to access files, applications, and web content from a single interface.

## Features

- **Floating Window**: Transparent, always-on-top search window that stays accessible while working
- **Plugin System**: Extensible architecture supporting custom plugins for different search capabilities
- **Real-time Search**: Instant search results as you type
- **Cross-platform**: Built with Flutter for Windows, macOS, and Linux support
- **Customizable Interface**: Settings panel and plugin management
- **Keyboard Controls**: Supports keyboard shortcuts and navigation

## Architecture

Magic Box is designed as a plugin-based search tool:
- **Main Application**: The Flutter UI that provides the search interface
- **Plugin System**: External executables that perform specific search tasks
- **Communication**: Plugins communicate with the main app via JSON over standard input/output
- **Plugin Format**: Plugins are distributed as ZIP packages containing executables and configurations

## Usage

1. Launch Magic Box - it appears as a small floating search bar
2. Start typing in the search box to begin searching
3. Results from enabled plugins will appear below the search bar
4. Click on results to execute their associated actions
5. Access settings via the gear icon to manage plugins and preferences
6. Close Magic Box with the close (X) button or press Escape
7. Use Ctrl+~ to open Magic Box anywhere for quick search

## Plugin Development

Magic Box supports a plugin system allowing developers to extend its functionality:

### Plugin Structure
```
plugin_name.zip
├── config.json
├── plugin_executable.exe
├── icon.png (optional)
└── other_files...
```

### Config.json Format
```json
{
  "sign": "mbep",
  "name": "Plugin Name",
  "version": "1.0.0",
  "main_file": "plugin_executable.exe",
  "icon": "icon.png" (optional)
}
```

### Plugin Communication Protocol
- Plugins receive search queries as command line arguments with the `-k` flag
- Plugins return results in JSON format through standard output
- Multiple results can be separated with the `next_result` delimiter

### JSON Result Format
```json
{
  "title": "Result Title",
  "content": "Result Content/Description",
  "cmd": "Command to execute when clicked",
  "preview_path": "Path to preview image (optional)",
  "encoding": "Text encoding (optional)",
  "auto_close": false // Optional: Whether to automatically close the window after opening the result
}
```

## Settings

Magic Box includes a settings interface to:
- Manage installed plugins
- Configure global hotkeys (coming soon)
- Adjust appearance and behavior

## Support

If you encounter any issues or have questions about Magic Box:

1. Check the [Issues](https://github.com/dmqj123/magic_box/issues) section
2. Report a new issue if your problem hasn't been addressed
3. Feel free to contribute to the project by forking and submitting pull requests