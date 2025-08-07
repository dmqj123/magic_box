import 'package:flutter/material.dart';
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
    size: Size(600, 80), // 设置窗口大小为800x80
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
  });

  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

bool is_more_spreadout = false;

class _MyAppState extends State<MyApp> with WindowListener {
  @override
  void initState() {
    super.initState();
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
              child: Row(
                children: [
                  Expanded(
                    child: TransparentSearchBox(),
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
                                      is_more_spreadout = true;
                                      setState(() {});
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
                                        },
                                        icon: const Icon(Icons.settings),
                                        style: IconButton.styleFrom(
                                          backgroundColor: const Color.fromARGB(255, 47, 96, 129),
                                          splashFactory: NoSplash.splashFactory,
                                        ),
                                      ),
                                      const SizedBox(width: 3),
                                      IconButton(
                                        onPressed: () {
                                          windowManager.close();
                                        },
                                        icon: const Icon(Icons.close),
                                        style: IconButton.styleFrom(
                                          backgroundColor: const Color.fromARGB(
                                            255,
                                            129,
                                            47,
                                            47,
                                          ),
                                          splashFactory: NoSplash.splashFactory,
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
            ),
          ),
        ),
      ),
    );
  }
}

class TransparentSearchBox extends StatefulWidget {
  const TransparentSearchBox({super.key});

  @override
  State<TransparentSearchBox> createState() => _TransparentSearchBoxState();
}

class _TransparentSearchBoxState extends State<TransparentSearchBox> {
  final TextEditingController _controller = TextEditingController();
  final FocusNode _focusNode = FocusNode();

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
              onTap: () {
                // 由于is_more_spreadout是全局变量，需要调用父级widget的setState来刷新UI
                setState(() {});
                is_more_spreadout = false;
                // 通知父级widget进行刷新
                (context.findAncestorStateOfType<_MyAppState>())?.setState(() {});
              },
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
    _focusNode.dispose();
    super.dispose();
  }
}
