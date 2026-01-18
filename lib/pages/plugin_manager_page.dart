import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:magic_box/class.dart' show Plugin;
import 'package:magic_box/service.dart';

class PluginManagerPage extends StatefulWidget {

  @override
  _PluginManagerPageState createState() => _PluginManagerPageState();
}

class _PluginManagerPageState extends State<PluginManagerPage> {
  Future<List<Plugin>>? _pluginsFuture;

  Future<List<Plugin>> _getplugins() async {
    return await getEnablePlugins();
  }

  @override
  void initState() {
    super.initState();
    _pluginsFuture = _getplugins();
  }

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
      body: FutureBuilder<List<Plugin>>(
        future: _pluginsFuture,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          } else if (snapshot.hasError) {
            return Center(child: Text('加载失败: ${snapshot.error}'));
          } else if (snapshot.hasData) {
            final plugins = snapshot.data!;
            return ListView.builder(
              itemCount: plugins.length,
              itemBuilder: (context, index) {
                final plugin = plugins[index];
                return Card(
                  margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                  child: ListTile(
                    title: Text(plugin.name),
                    subtitle: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        //Text(plugin.description), 描述
                      ],
                    ),
                    trailing: IconButton(
                      onPressed: () {
                        try {
                          delPlugin(plugin.name);
                          setState(() {
                            plugins.remove(plugin);
                          });
                          ScaffoldMessenger.of(context).showSnackBar(
                            SnackBar(
                              content: Text('插件已删除'),
                              duration: const Duration(seconds: 1),
                            ),
                          );
                        } catch (e) {
                          //弹出消息
                          showDialog(
                            context: context,
                            builder: (BuildContext context) {
                              return AlertDialog(
                                title: Text('提示'),
                                content: Text('删除失败'),
                                actions: <Widget>[
                                  TextButton(
                                    child: Text('确定'),
                                    onPressed: () {
                                      Navigator.of(context).pop();
                                    },
                                  ),
                                ],
                              );
                            },
                          );
                        }
                      },
                      icon: Icon(Icons.delete),
                    ),
                  ),
                );
              },
            );
          } else {
            return const Center(child: Text('暂无数据'));
          }
        },
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () async {
          // TODO: 实现添加新插件的功能
          //用file_picker选择文件
          FilePickerResult? result = await FilePicker.platform.pickFiles(
            type: FileType.custom,
            allowedExtensions: ['zip'],
          );
          
          if (result != null) {
            // 获取选择的文件路径
            String path = result.files.single.path!;
            // 添加插件
            await addPlugin(path);
            // 重新加载插件列表
            setState(() {
              _pluginsFuture = _getplugins();
            });
            ScaffoldMessenger.of(context).showSnackBar(
              SnackBar(
                content: Text('插件已添加'),
                duration: const Duration(seconds: 1),
              ),
            );
          }
        },
        child: const Icon(Icons.add),
      ),
    );
  }
}