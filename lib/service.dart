import 'dart:convert';
import 'dart:io';

import 'components.dart';
import 'class.dart';

List<plugin> plugins = [
  //TEST
  plugin(
    name: "FileSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\FileSearch\\Debug\\FileSearch.exe",
    version: "1.0.0",
    icon_path: "C:\\Users\\abcdef\\Downloads\\robot.png",
  ),
];

Future<List<ResultItemCard>> getResultItems(String query) async {
  List<ResultItemCard> result_list = [];
  List<Future<List<ResultItemCard>?>> futures = [];

  for (plugin item in plugins) {
    futures.add(Future<List<ResultItemCard>?> (() async {
      Process process = await Process.start("cmd", [
        "/C " + item.path + " -k " + query,
      ]);

      final output =
          await process.stdout.transform(systemEncoding.decoder).join();
      final results = AddResultItemCardFromJson(output);

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

  return result_list;
}

List<ResultItemCard>? AddResultItemCardFromJson(String jsonString) {
  List<ResultItemCard> allResults = [];
  List<String> json_list = jsonString.split("\nnext_result");

  for (String json_item in json_list) {
    if (json_item.trim().isEmpty) continue; // Skip empty strings

    try {
      final decodedJson = jsonDecode(json_item);

      if (decodedJson is List) {
        allResults.addAll(decodedJson.map<ResultItemCard>((item) {
          return ResultItemCard(
            title: item['name']?.toString() ?? '未命名',
            content: item['path']?.toString() ?? '路径未知',
            cmd: item['open_cmd']?.toString(),
            image_path: item['icon_path']?.toString(),
          );
        }).toList());
      } else if (decodedJson is Map<String, dynamic>) {
        allResults.add(
          ResultItemCard(
            title: decodedJson['name']?.toString() ?? '未命名',
            content: decodedJson['path']?.toString() ?? '路径未知',
            cmd: decodedJson['open_cmd']?.toString(),
            image_path: decodedJson['icon_path']?.toString(),
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
