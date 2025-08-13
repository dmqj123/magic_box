import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'package:magic_box/components.dart';
import 'package:magic_box/pages.dart';
import 'package:magic_box/service.dart';
import 'package:magic_box/const.dart';

import 'package:window_manager/window_manager.dart';

void main() async {
  // 确保Flutter绑定初始化完成
  WidgetsFlutterBinding.ensureInitialized();
  // Must add this line.
  // 确保窗口管理器初始化完成
  await windowManager.ensureInitialized();

  // 定义窗口选项配置
  WindowOptions windowOptions = const WindowOptions(
    minimumSize: Size(100, 60),
    size: Size(WINDOW_WIDTH, 80), // 设置窗口大小为800x80
    center: true, // 窗口居中显示
    backgroundColor: Colors.transparent, // 窗口背景透明
    skipTaskbar: false, // 在任务栏显示窗口
    titleBarStyle: TitleBarStyle.hidden, // 隐藏标题栏
  );
  // 等待窗口准备就绪后显示和聚焦
  windowManager.waitUntilReadyToShow(windowOptions, () async {
    windowManager.show(); // 显示窗口
    windowManager.focus(); // 窗口获取焦点
    windowManager.setAsFrameless();
    windowManager.setAlwaysOnTop(true);
    windowManager.setResizable(false);
  });

  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

 late final _MyAppState myAppState;
class _MyAppState extends State<MyApp> with WindowListener {
  bool is_more_spreadout = false;
  bool is_result_show = false;
  String input_text = "";

  void getResults() async {
    result_items = await getResultItems(input_text,onDataChange: (data){
      setState(() {
        if(data!=null){
          result_items = data;
        }
      });
    });
    setState(() {});
  }

  @override
  void initState() {
    super.initState();
    myAppState = this;

    windowManager.addListener(this);
  }

  @override
  void dispose() {
    windowManager.removeListener(this);
    super.dispose();
  }

  @override
  void onWindowMaximize() {
    windowManager.unmaximize();
  }

  @override
  void onWindowMinimize() {
    windowManager.show();
  }

  @override
  Widget build(BuildContext context) {
    if (is_result_show) {
      double height = 120;
      //根据内容判断高度，最大500
      height += result_items.length * 80;
      if (height > 500) {
        height = 500;
      }
      if (result_items.length == 0) {
        height = 150;
      }
      windowManager.setSize(Size(WINDOW_WIDTH, height));
    } else {
      windowManager.setSize(Size(WINDOW_WIDTH, 80));
    }
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark(), // 使用暗色主题更易看清透明效果
      home: Scaffold(
        backgroundColor: Colors.transparent, // 设置Scaffold背景透明
        extendBodyBehindAppBar: true, // 允许body延伸到AppBar后面
        extendBody: true, // 允许body延伸到底部导航栏后面
        body: DragToMoveArea(
          child: Center(
            child: Padding(
              padding: const EdgeInsets.symmetric(horizontal: 20.0),
              child: Column(
                children: [
                  Row(
                    //输入框部分
                    children: [
                      Expanded(
                        child: TransparentSearchBox(
                          onSearchSubmitted: (value) {
                            if (value.isNotEmpty) {
                              setState(() {
                                is_result_show = true;
                                input_text = value;
                              });
                            }
                          },
                          onTap: () {
                            setState(() {
                              is_more_spreadout = false;
                            });
                          },
                        ),
                      ), // 将TransparentSearchBox包裹在Expanded中
                      SizedBox(width: 3),
                      Container(
                        padding: const EdgeInsets.all(4.0), // 添加内边距
                        decoration: BoxDecoration(
                          border: Border.all(
                            color: Colors.white.withOpacity(0.5),
                            width: 1.0,
                          ),
                          borderRadius: BorderRadius.circular(23),
                        ),
                        child: Row(
                          children: [
                            AnimatedSwitcher(
                              duration: const Duration(milliseconds: 150),
                              transitionBuilder: (
                                Widget child,
                                Animation<double> animation,
                              ) {
                                return ScaleTransition(
                                  scale: animation,
                                  child: child,
                                );
                              },
                              child:
                                  !is_more_spreadout
                                      ? IconButton(
                                        key: const ValueKey<bool>(true),
                                        onPressed: () {
                                          setState(() {
                                            is_more_spreadout = true;
                                          });
                                        },
                                        icon: const Icon(Icons.more_horiz),
                                        style: IconButton.styleFrom(
                                          backgroundColor: const Color.fromARGB(
                                            255,
                                            85,
                                            85,
                                            85,
                                          ),
                                          splashFactory: NoSplash.splashFactory,
                                        ),
                                      )
                                      : Row(
                                        key: const ValueKey<bool>(false),
                                        children: [
                                          IconButton(
                                            onPressed: () {
                                              //打开设置
                                              /*
                                              Navigator.push(
                                                context,
                                                MaterialPageRoute(
                                                  builder: (context) =>
                                                      const SettingsPage(),
                                                ),
                                              );*/
                                            },
                                            icon: const Icon(Icons.settings),
                                            style: IconButton.styleFrom(
                                              backgroundColor:
                                                  const Color.fromARGB(
                                                    255,
                                                    47,
                                                    96,
                                                    129,
                                                  ),
                                              splashFactory:
                                                  NoSplash.splashFactory,
                                            ),
                                          ),
                                          const SizedBox(width: 3),
                                          IconButton(
                                            onPressed: () {
                                              windowManager.close();
                                            },
                                            icon: const Icon(Icons.close),
                                            style: IconButton.styleFrom(
                                              backgroundColor:
                                                  const Color.fromARGB(
                                                    255,
                                                    129,
                                                    47,
                                                    47,
                                                  ),
                                              splashFactory:
                                                  NoSplash.splashFactory,
                                            ),
                                          ),
                                          /*
                                      const SizedBox(width: 3),
                                      IconButton(
                                        onPressed: () {
                                          is_more_spreadout = false;
                                          setState(() {});
                                        },
                                        icon: const Icon(Icons.chevron_left),
                                        style: IconButton.styleFrom(
                                          backgroundColor: const Color.fromARGB(
                                            255,
                                            47,
                                            129,
                                            81,
                                          ),
                                          splashFactory: NoSplash.splashFactory,
                                        ),
                                      ),*/
                                        ],
                                      ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                  if (is_result_show)
                    Expanded(
                      child: Column(
                        children: [
                          SizedBox(height: 10),
                          Expanded(child: _build_result_view()),
                        ],
                      ),
                    ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _build_result_view() {
    return Container(
      decoration: BoxDecoration(
        color: const Color.fromARGB(232, 107, 107, 107),
        borderRadius: BorderRadius.circular(15.0), // 添加圆角效果
      ),
      child: Padding(
        padding: const EdgeInsets.all(10.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch, // 使子组件宽度占满
          children: [
            Row(
              children: [
                Text(
                  "结果：",
                  style: TextStyle(fontSize: 22, fontWeight: FontWeight.bold),
                ),
                Expanded(child: SizedBox(width: 1)),
                IconButton(
                  onPressed: () {
                    setState(() {
                      //TODO
                      setState(() {
                        getResults();
                      });
                    });
                  },
                  icon:
                      (is_getting_result)
                          ? SizedBox(
                              width: 23,
                              height: 23,
                              child: CircularProgressIndicator(),
                            )
                          : Icon(Icons.replay),
                ),
                IconButton(
                  onPressed: () {
                    setState(() {
                      is_result_show = false;
                    });
                  },
                  icon: Icon(Icons.arrow_upward),
                ),
              ],
            ),
            Expanded(
              child: ListView(

                children: [
                  if (result_items.length == 0) Text(is_getting_result ? "获取中..." : "暂无结果"),

                  ...result_items,
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

class TransparentSearchBox extends StatefulWidget {
  final ValueChanged<String> onSearchSubmitted;
  final VoidCallback onTap;

  const TransparentSearchBox({
    super.key,
    required this.onSearchSubmitted,
    required this.onTap,
  });

  @override
  State<TransparentSearchBox> createState() => _TransparentSearchBoxState();
}

class _TransparentSearchBoxState extends State<TransparentSearchBox> {
  final TextEditingController _controller = TextEditingController();
  final FocusNode _focusNode = FocusNode();

  @override
  void initState() {
    super.initState();
    _focusNode.addListener(_onFocusChanged);
    // 在窗口加载时自动获取焦点
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _focusNode.requestFocus();
    });
  }

  void _onFocusChanged() {
    if (_focusNode.hasFocus) {
      RawKeyboard.instance.addListener(_handleKeyEvent);
    } else {
      RawKeyboard.instance.removeListener(_handleKeyEvent);
    }
  }

  void _handleKeyEvent(RawKeyEvent event) {
    if (event is RawKeyDownEvent &&
        event.logicalKey == LogicalKeyboardKey.escape) {
      windowManager.close();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.86), // 半透明黑色背景
        borderRadius: BorderRadius.circular(30.0),
        border: Border.all(color: Colors.white.withOpacity(0.5), width: 1.0),
      ),
      child: Row(
        children: [
          const Padding(
            padding: EdgeInsets.only(left: 16.0, right: 8.0),
            child: Icon(Icons.arrow_forward, color: Colors.white70, size: 24),
          ),
          Expanded(
            child: TextField(
              onSubmitted: widget.onSearchSubmitted,
              //当文字改变
              onChanged: (value) {
                myAppState!.setState(() {
                  myAppState!.is_result_show = true;
                  myAppState!.input_text = value;
                  if (value != "") {
                    myAppState!.getResults();
                  }
                });
              },

              controller: _controller,
              focusNode: _focusNode,
              style: const TextStyle(color: Colors.white),
              decoration: const InputDecoration(
                hintText: '请输入内容内容...',
                hintStyle: TextStyle(color: Colors.white70),
                border: InputBorder.none, // 移除默认下划线
                contentPadding: EdgeInsets.symmetric(vertical: 16.0),
              ),
              cursorColor: Colors.white,
              onTap: widget.onTap,
            ),
          ),
          if (_controller.text.isNotEmpty)
            IconButton(
              icon: const Icon(Icons.clear, color: Colors.white70),
              onPressed: () => _controller.clear(),
            ),
        ],
      ),
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    _focusNode.removeListener(_onFocusChanged);
    _focusNode.dispose();
    RawKeyboard.instance.removeListener(_handleKeyEvent);
    super.dispose();
  }
}
