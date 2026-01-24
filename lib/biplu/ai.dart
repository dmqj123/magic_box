import 'dart:async';
import 'dart:convert';
import 'package:http/http.dart' as http;
import 'package:shared_preferences/shared_preferences.dart';
import 'package:magic_box/components.dart' show ResultItemCard;
import 'package:magic_box/main.dart' show myAppState;

AiInfo aiInfo = AiInfo();

Future<void> loadAiInfo() async {
  final prefs = await SharedPreferences.getInstance();
  aiInfo.api_url = prefs.getString('ai_url');
  aiInfo.api_key = prefs.getString('ai_key');
  aiInfo.model = prefs.getString('ai_model');
}

class AiAbility {
  final http.Client _httpClient = http.Client();
  bool _isAiInfoLoaded = false;

  Future<void> _ensureAiInfoLoaded() async {
    if (!_isAiInfoLoaded) {
      await loadAiInfo();
      _isAiInfoLoaded = true;
    }
  }

  Future<String?> _callAiApi(String query) async {
    await _ensureAiInfoLoaded();

    if (aiInfo.api_url == null ||
        aiInfo.api_url!.isEmpty ||
        aiInfo.api_key == null ||
        aiInfo.api_key!.isEmpty ||
        aiInfo.model == null ||
        aiInfo.model!.isEmpty) {
      return null;
    }

    try {
      print('调用AI API: $query');
      if (query.length >= 900) {
        return null;
      }
      final url = Uri.parse('${aiInfo.api_url}/chat/completions');

      final response = await _httpClient.post(
        url,
        headers: {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer ${aiInfo.api_key}',
        },
        body: jsonEncode({
          'model': aiInfo.model,
          'messages': [
            {
              'role': 'system',
              'content':
                  """你是一个搜索引擎中的AI助手，当用户搜索一些官网的名字时，你会给出推荐的官网网址，当用户搜索到一些外国语时，你可以给出一些中文翻译，如果用户搜索到一些百科或者词汇，你还可以给出相应的解释。若Ai搜索的是无关内容（不是外语、网站、百科），请回复null
请以这个格式输出内容，并不要添加任何其他成分（除非输出为null）：
{
  "title": "结果标题",
  "content": "结果内容/描述",
  "cmd": "单击时执行的命令（cmd命令，一般不要添加，除了要跳转网址的可以写上explorer http://example.com）",
}
""",
            },
            {'role': 'user', 'content': query},
          ],
        }),
      ).timeout(
        Duration(seconds: 8),
        onTimeout: () {
          print('AI 请求超时');
          throw TimeoutException('AI 请求超时');
        },
      );
      if (response.statusCode == 200) {
        print('AI响应状态码: ${response.statusCode}');
        print('AI响应体: ${response.body}');
        final data = jsonDecode(response.body);
        if (data['choices'] != null && data['choices'].isNotEmpty) {
          String answer = data['choices'][0]['message']['content'] as String;
          if (answer == 'null') {
            return null;
          } else {
            return answer;
          }
        }
      }
      return null;
    } on TimeoutException catch (e) {
      print('AI 请求超时: $e');
      return null;
    } catch (e) {
      print('调用AI API出错: $e');
      return null;
    }
  }

  Future<List<ResultItemCard>> getAiResults(String query) async {
    List<ResultItemCard> results = [];

    if (query == "配置Ai插件") {
      return [
        ResultItemCard(
          title: "配置Ai插件",
          content: "配置Ai插件的接口，确保Ai功能正常使用",
          onTap: () {
            Change_ai_info();
          },
        ),
      ];
    }
    //调用Ai
    final response = await _callAiApi(query);
    if (response == null) {
      return results;
    }
    try {
      print('AI响应: $response');
      final data = jsonDecode(response);
      if (data['title'] != null &&
          data['content'] != null &&
          data['cmd'] != null) {
        results.add(
          ResultItemCard(
            title: data['title'],
            content: data['content'],
            cmd: data['cmd'],
          ),
        );
      }
    } catch (e) {
      print('解析AI响应出错: $e');
    }
    return results;
  }

  void Change_ai_info() {
    myAppState.navigateToAiSettings();
  }

  Future<void> refreshAiConfig() async {
    _isAiInfoLoaded = false;
    await _ensureAiInfoLoaded();
  }

  void cancelRequest() {
    _isAiInfoLoaded = false;
  }
}

class AiInfo {
  String? api_url;
  String? api_key;
  String? model;
}
