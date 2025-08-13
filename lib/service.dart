import 'dart:convert';
import 'dart:io';

import 'components.dart';
import 'class.dart';

bool is_getting_result = false;
List<Process> get_result_processes = [];
List<ResultItemCard> result_items = [];

List<plugin> plugins = [
  //TEST
  plugin(
    name: "FileSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\FileSearch\\x64\\Debug\\FileSearch.exe",
    version: "1.0.0",
    icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),
  plugin(
    name: "AppSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\AppSearch\\Debug\\AppSearch.exe",
    version: "1.0.0",
    //icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),
  /*
  plugin(
    name: "WebSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\WebSearch\\Debug\\WebSearch.exe",
    version: "1.0.0",
    //icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),*/

];

List<plugin> getPlugins() {
  //TODO 从配置文件读取插件
  return plugins;
}

void killAllRunningProcesses() {
  for (var process in get_result_processes) {
    try {
      process.kill();
      print('进程 ${process.pid} 已被终止');
    } catch (e) {
      print('终止进程 ${process.pid} 失败: $e');
    }
  }
  get_result_processes.clear(); // 清空列表
}

Future<List<ResultItemCard>> getResultItems(String query, {void Function(List<ResultItemCard>?)? onDataChange}) async {
  killAllRunningProcesses();

  is_getting_result = true;

  List<ResultItemCard> result_list = [];
  List<Future<List<ResultItemCard>?>> get_results_futures = [];

  for (plugin item in plugins) {
    get_results_futures.add(Future<List<ResultItemCard>?> (() async {
      get_result_processes.add(await Process.start("cmd", [
        "/C " + item.path + " -k " + query,
      ]));
      Process process = get_result_processes.last;

      // 从进程的标准输出读取数据并解码
      // 从进程的标准输出实时读取数据并解码
      List<ResultItemCard>? results = [];
      /*
      process.stdout.transform(systemEncoding.decoder).listen((data) {
        print("object");
        // 每当有新的数据块可用时，此回调函数就会被调用
        // 假设每次接收到的 `data` 都是一个可以被 `AddResultItemCardFromJson` 处理的完整或部分JSON字符串
        // 如果 `data` 不是完整的JSON，这里可能会抛出解析异常，您可能需要累积数据后再解析
        results = AddResultItemCardFromJson(data,item.icon_path);
        // 您可以在这里处理实时结果，例如更新UI
        print('实时结果: $data');
      });*/
      //获取全部结果
      List<int> stdout_bytes = (await process.stdout.toList()).expand((bytes) => bytes).toList();
       String stdout_str;
       try {
         stdout_str = utf8.decode(stdout_bytes, allowMalformed: true);
       } on FormatException catch (e) {
         print('UTF-8解码错误: $e');
         /*print('原始字节数据 (部分): ${stdout_bytes.sublist(0, stdout_bytes.length > 256 ? 256 : stdout_bytes.length)}');*/ // 打印前256个字节或全部
         rethrow; // 重新抛出异常，以便上层捕获
       }
       results = AddResultItemCardFromJson(stdout_str, item.icon_path);
      // 等待进程结束并获取退出码
      final exitCode = await process.exitCode;
      onDataChange?.call(results);
      print("${item.name}插件查询完成，退出码：$exitCode");
      // 将非空的results中的所有项添加到result_list
      // 移除此处对 result_list 的修改
      // if (results != null && results.isNotEmpty) {
      //   result_list.addAll(results);
      // }
      return results; // 返回当前 Future 获得的结果
    }));
  }

  List<List<ResultItemCard>?> allResults = await Future.wait(get_results_futures);

  for (List<ResultItemCard>? results in allResults) {
    if (results != null) {
      result_list.addAll(results);
    }
  }
  is_getting_result = false;
  return result_list;
}

List<ResultItemCard>? AddResultItemCardFromJson(String jsonString, String? plugin_image_path) {

  List<ResultItemCard> allResults = [];
  List<String> json_list = jsonString.split("next_result");

  for (String json_item in json_list) {
    if (json_item.trim().isEmpty) continue; // Skip empty strings

    try {
      final decodedJson = jsonDecode(json_item);

      if (decodedJson is List) {
        allResults.addAll(decodedJson.map<ResultItemCard>((item) {
          return ResultItemCard(
            title: item['title']?.toString() ?? '未命名',
            content: item['content']?.toString().replaceAll('\\', '\\') ?? '路径未知',
            cmd: item['cmd']?.toString().replaceAll('\\', '\\'),
            image_path: plugin_image_path,
            preview_path: item['preview_path']?.toString().replaceAll('\\', '\\'),
          );
        }).toList());
      } else if (decodedJson is Map<String, dynamic>) {

        allResults.add(
          ResultItemCard(
            title: decodedJson['title']?.toString() ?? '未命名',
            content: decodedJson['content']?.toString().replaceAll('\\', '\\') ?? '路径未知',
            cmd: decodedJson['cmd']?.toString().replaceAll('\\', '\\'),
            image_path: plugin_image_path,
            preview_path: decodedJson['preview_path']?.toString().replaceAll('\\', '\\'),
          ),
        );

      } else {
        print('JSON解析错误: 未知的JSON类型 - $json_item');
      }
    } catch (e) {
      print('JSON解析错误: $e for input: $json_item');
    }
  }
  return allResults.isNotEmpty ? allResults : null;
}
