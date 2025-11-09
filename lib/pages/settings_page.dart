import 'package:flutter/material.dart';
import 'package:magic_box/settings.dart';
import 'package:magic_box/pages/plugin_manager_page.dart';

class SettingsPage extends StatefulWidget {
  const SettingsPage({super.key});

  @override
  State<SettingsPage> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('设置'),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => Navigator.of(context).pop(),
        ),
      ),
      body: ListView(
        children: [
          // 快捷键设置
          ListTile(
            leading: const Icon(Icons.keyboard),
            title: const Text('全局快捷键'),
            subtitle: Text(Settings.global_shortcut ?? '未设置'),
            onTap: () {
              // TODO: 实现快捷键设置
            },
          ),
          const Divider(),
          // 插件管理
          ListTile(
            leading: const Icon(Icons.extension),
            title: const Text('插件管理'),
            subtitle: const Text('管理已安装的插件'),
            trailing: const Icon(Icons.arrow_forward_ios),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => const PluginManagerPage(),
                ),
              );
            },
          ),
          const Divider(),
          // 其他设置项...
        ],
      ),
    );
  }
}
