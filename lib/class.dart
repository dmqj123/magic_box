class Plugin {
  String name;
  String path;
  String version;
  String? icon_path;

  // ToJson方法
  Map<String, dynamic> toJson() {
    return {
      'name': name,
      'path': path,
      'version': version,
      'icon_path': icon_path,
    };
  }

  // 从JSON创建Plugin实例的工厂构造函数
   factory Plugin.fromJson(Map<String, dynamic> json) {
     return Plugin(
       name: json['name'] as String,
       path: json['path'] as String,
       version: json['version'] as String,
       icon_path: json['icon_path'] as String?,
     );
   }

  Plugin({
    required this.name,
    required this.path,
    required this.version,
    this.icon_path,
  });
}