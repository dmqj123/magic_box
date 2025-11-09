import 'package:flutter/material.dart';

class PluginManagerPage extends StatefulWidget {
  const PluginManagerPage({super.key});

  @override
  State<PluginManagerPage> createState() => _PluginManagerPageState();
}

class _PluginManagerPageState extends State<PluginManagerPage> {
  final List<Plugin> _plugins = [
    // 示例插件数据
    Plugin(
      name: 'App搜索',
      description: 'Windows应用搜索插件',
      enabled: true,
      path: 'plugins_dev/AppSearch',
    ),
    Plugin(
      name: '文件搜索',
      description: '本地文件搜索插件',
      enabled: true,
      path: 'plugins_dev/FileSearch',
    ),
    Plugin(
      name: '网页搜索',
      description: '网页内容搜索插件',
      enabled: true,
      path: 'plugins_dev/WebSearch',
    ),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('插件管理'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => Navigator.of(context).pop(),
        ),
      ),
      body: ListView.builder(
        itemCount: _plugins.length,
        itemBuilder: (context, index) {
          final plugin = _plugins[index];
          return Card(
            margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            child: ListTile(
              title: Text(plugin.name),
              subtitle: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(plugin.description),
                  Text(
                    '路径: ${plugin.path}',
                    style: TextStyle(
                      fontSize: 12,
                      color: Theme.of(context).textTheme.bodySmall?.color,
                    ),
                  ),
                ],
              ),
              trailing: Switch(
                value: plugin.enabled,
                onChanged: (value) {
                  setState(() {
                    plugin.enabled = value;
                    // TODO: 实现插件启用/禁用逻辑
                  });
                },
              ),
            ),
          );
        },
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () {
          // TODO: 实现添加新插件的功能
        },
        child: const Icon(Icons.add),
      ),
    );
  }
}

class Plugin {
  String name;
  String description;
  bool enabled;
  String path;

  Plugin({
    required this.name,
    required this.description,
    required this.enabled,
    required this.path,
  });
}
