import 'dart:convert';
import 'dart:io';

import 'components.dart';
import 'class.dart';

// 根据编码名称获取编码对象
Encoding getEncodingByName(String? encodingName) {
  if (encodingName == null) {
    return utf8; // 默认使用UTF-8编码
  }

  switch (encodingName.toLowerCase()) {
    case 'utf8':
    case 'utf-8':
      return utf8;
    case 'ascii':
      return ascii;
    case 'latin1':
      return latin1;
    // 可以根据需要添加更多编码
    default:
      return utf8; // 默认使用UTF-8编码
  }
}

// 默认系统编码
final Encoding systemEncoding = utf8;

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
    //icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),
  plugin(
    name: "AppSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\AppSearch\\Debug\\AppSearch.exe",
    version: "1.0.0",
    //icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),
  
  plugin(
    name: "WebSearch",
    path:
        "J:\\zzx\\Code\\Flutter\\magic_box\\plugins_dev\\WebSearch\\Debug\\WebSearch.exe",
    version: "1.0.0",
    //icon_path: "C:\\Users\\abcdef\\Downloads\\aQjQWax4Tl.jpg",
  ),
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

Future<List<ResultItemCard>> getResultItems(
  String query, {
  void Function(List<ResultItemCard>?)? onDataChange,
}) async {
  killAllRunningProcesses();

  is_getting_result = true;

  List<ResultItemCard> result_list = [];
  List<Future<List<ResultItemCard>?>> get_results_futures = [];

  for (plugin item in plugins) {
    get_results_futures.add(
      Future<List<ResultItemCard>?>(() async {
        get_result_processes.add(
          await Process.start("cmd", ["/C " + item.path + " -k " + query]),
        );
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
        // 首先读取进程的输出 as raw bytes to handle encoding properly
        List<int> rawOutputBytes = [];
        await for (List<int> chunk in process.stdout) {
          rawOutputBytes.addAll(chunk);
        }
        
        String rawOutput;
        try {
          // Try to decode as UTF-8 first
          rawOutput = utf8.decode(rawOutputBytes);
        } catch (e) {
          // If UTF-8 fails, try other encodings or handle error gracefully
          print('UTF-8 decoding failed: $e');
          try {
            // If you're on a Chinese system, you might want to try GBK encoding
            // Since Dart doesn't have GBK built-in, we would use the convert package
            // For now, we'll try Latin1 as a fallback which preserves the bytes
            rawOutput = latin1.decode(rawOutputBytes);
          } catch (e2) {
            print('Fallback encoding also failed: $e2');
            // As a last resort, try to decode with error handling
            rawOutput = utf8.decode(rawOutputBytes, allowMalformed: true);
          }
        }

        // 尝试解析JSON以获取编码信息
        String? encodingFromJson;
        try {
          // 只在rawOutput不为空时尝试解析JSON
          if (rawOutput.trim().isNotEmpty) {
            final decodedJson = jsonDecode(rawOutput.split("next_result")[0]);
            if (decodedJson is Map<String, dynamic>) {
              encodingFromJson = decodedJson['encoding']?.toString();
            } else if (decodedJson is List && decodedJson.isNotEmpty) {
              final firstItem = decodedJson[0];
              if (firstItem is Map<String, dynamic>) {
                encodingFromJson = firstItem['encoding']?.toString();
              }
            }
          }
        } catch (e) {
          print('解析JSON获取编码信息时出错: $e');
        }

        // 根据JSON中的编码信息或默认编码来解码输出
        // 不再重新监听process.stdout，而是使用已经读取的rawOutput
        Encoding encoding = getEncodingByName(encodingFromJson);
        // 如果编码不同，需要重新解码
        if (encoding != systemEncoding) {
          // 这里我们假设插件输出的是UTF-8编码，但标记为其他编码
          // 在实际应用中，可能需要更复杂的处理
          // 暂时我们直接使用rawOutput
        }

        results = AddResultItemCardFromJson(
          rawOutput,
          item.icon_path,
          encodingFromJson, // 传递从JSON中获取的编码信息
        );
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
      }),
    );
  }

  List<List<ResultItemCard>?> allResults = await Future.wait(
    get_results_futures,
  );

  for (List<ResultItemCard>? results in allResults) {
    if (results != null) {
      result_list.addAll(results);
    }
  }
  is_getting_result = false;
  return result_list;
}

List<ResultItemCard>? AddResultItemCardFromJson(
  String jsonString,
  String? plugin_image_path,
  String? encoding,
) {
  List<ResultItemCard> allResults = [];

  List<String> json_list = jsonString.split("next_result");

  for (String json_item in json_list) {
    if (json_item.trim().isEmpty) continue; // Skip empty strings

    try {
      final decodedJson = jsonDecode(json_item);

      if (decodedJson is List) {
        allResults.addAll(
          decodedJson.map<ResultItemCard>((item) {
            return ResultItemCard(
              title: item['title']?.toString() ?? '未命名',
              content: item['content']?.toString() ?? '路径未知',
              cmd: item['cmd']?.toString(),
              image_path: plugin_image_path,
              preview_path: item['preview_path']?.toString(),
              encoding:
                  item['encoding']?.toString() ??
                  encoding, // 如果没有encoding键，则使用传入的编码
            );
          }).toList(),
        );
      } else if (decodedJson is Map<String, dynamic>) {
        allResults.add(
          ResultItemCard(
            title: decodedJson['title']?.toString() ?? '未命名',
            content: decodedJson['content']?.toString() ?? '路径未知',
            cmd: decodedJson['cmd']?.toString(),
            image_path: plugin_image_path,
            preview_path: decodedJson['preview_path']?.toString(),
            encoding:
                decodedJson['encoding']?.toString() ??
                encoding, // 如果没有encoding键，则使用传入的编码
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
