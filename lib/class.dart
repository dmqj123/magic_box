class Plugin{
  String name;
  String path;
  String version;
  String? icon_path;

  //ToJson方法
  Map<String, dynamic> toJson() {
    return {
      'name': name,
      'path': path,
      'version': version,
      'icon_path': icon_path,
    };
  }

  Plugin({required this.name,required this.path,required this.version, this.icon_path});
}