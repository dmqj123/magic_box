import 'dart:convert';
import 'dart:io';

import 'components.dart';
import 'class.dart';

bool is_getting_result = false;

List<plugin> plugins = [
  //TEST
  plugin(
    name: "FileSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\FileSearch\\Debug\\FileSearch.exe",
    version: "1.0.0",
    //icon_path: "C:\\Users\\abcdef\\Downloads\\robot.png",
  ),
];

Future<List<ResultItemCard>> getResultItems(String query) async {
  is_getting_result = true;

  List<ResultItemCard> result_list = [];
  List<Future<List<ResultItemCard>?>> futures = [];

  for (plugin item in plugins) {
    futures.add(Future<List<ResultItemCard>?> (() async {
      Process process = await Process.start("cmd", [
        "/C " + item.path + " -k " + query,
      ]);

      final output =
          await process.stdout.transform(systemEncoding.decoder).join();
      final results = AddResultItemCardFromJson(output,item.icon_path);

      final exitCode = await process.exitCode;
      print("${item.name}插件查询完成，退出码：$exitCode");
      return results;
    }));
  }

  List<List<ResultItemCard>?> allResults = await Future.wait(futures);

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
  List<String> json_list = jsonString.split("\nnext_result");

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
          );
        }).toList());
      } else if (decodedJson is Map<String, dynamic>) {

        allResults.add(
          ResultItemCard(
            title: decodedJson['title']?.toString() ?? '未命名',
            content: decodedJson['content']?.toString().replaceAll('\\', '\\') ?? '路径未知',
            cmd: decodedJson['cmd']?.toString().replaceAll('\\', '\\'),
            image_path: plugin_image_path,

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
