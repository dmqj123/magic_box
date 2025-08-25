import 'dart:io';

import 'package:flutter/material.dart';
import 'package:magic_box/const.dart';

class ResultItemCard extends StatefulWidget {
  final String content;
  final String? title;
  final String? image_path;
  final VoidCallback? onTap;
  final String? preview_path;
  final String? cmd;
  final String? encoding;

  const ResultItemCard({
    Key? key,
    required this.content,
    this.title,
    this.image_path,
    this.preview_path,
    this.onTap,
    this.cmd,
    this.encoding,
  }) : super(key: key);

  @override
  State<ResultItemCard> createState() => _ResultItemCardState();
}

class _ResultItemCardState extends State<ResultItemCard> {
  late FocusNode _focusNode;
  bool _hasFocus = false;

  @override
  void initState() {
    super.initState();
    _focusNode = FocusNode();
    _focusNode.addListener(_onFocusChange);
  }

  void _onFocusChange() {
    setState(() {
      _hasFocus = _focusNode.hasFocus;
    });
  }

  @override
  void dispose() {
    _focusNode.removeListener(_onFocusChange);
    _focusNode.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.all(3.0),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(15.0)),
      color:
          (_hasFocus)
              ? const Color.fromARGB(255, 168, 168, 168)
              : Color.fromARGB(255, 233, 233, 233),
      elevation: 8,
      child: Focus(
        focusNode: _focusNode,
        onFocusChange: (hasFocus) {
          // _onFocusChange 已经处理了状态更新，这里可以留空或添加其他逻辑
        },
        child: InkWell(
          onTap: () {
            if (widget.onTap != null) {
              widget.onTap!();
            }
            //Process.run('cmd', ['/C', widget.cmd!]);
            Process.run('powershell', [widget.cmd!]);
          },
          borderRadius: BorderRadius.circular(15.0),
          child: Padding(
            padding: const EdgeInsets.all(6),
            child: Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (widget.image_path != null)
                  Padding(
                    padding: const EdgeInsets.only(right: 16.0),
                    child: ClipOval(
                      child: Image.file(
                        File(widget.image_path!),
                        width: 25,
                        height: 25,
                        fit: BoxFit.cover,
                        errorBuilder: (context, error, stackTrace) {
                          return Container(
                            width: 60,
                            height: 60,
                            color: Colors.grey[200],
                            child: Icon(
                              Icons.broken_image,
                              color: Colors.grey[400],
                              size: 25, // 指定图标大小与图片大小一致
                            ),
                          );
                        },
                      ),
                    ),
                  ),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      if (widget.title != null)
                        Padding(
                          padding: const EdgeInsets.only(bottom: 8.0),
                          child: Text(
                            widget.title!,
                            style: const TextStyle(
                              color: Colors.black,
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                        ),
                      Text(
                        widget.content,
                        style: const TextStyle(
                          fontSize: 16,
                          color: Colors.black54,
                        ),
                      ),
                    ],
                  ),
                ),
                if (widget.preview_path != null &&
                    widget.preview_path!.isNotEmpty)
                  ClipRRect(
                    borderRadius: BorderRadius.circular(8.0),
                    child: widget.preview_path!.startsWith('http://') || 
                          widget.preview_path!.startsWith('https://') 
                        ? Image.network(
                            widget.preview_path!,
                            width: WINDOW_WIDTH / 12,
                            height: WINDOW_WIDTH / 12,
                            fit: BoxFit.cover,
                          )
                        : Image.file(
                            File(widget.preview_path!),
                            width: WINDOW_WIDTH / 12,
                            height: WINDOW_WIDTH / 12,
                            fit: BoxFit.cover,
                          ),
                  ),
                if (_hasFocus)
                  Center(
                    child: Icon(
                      Icons.keyboard_return,
                      color: const Color.fromARGB(255, 83, 83, 83),
                    ),
                  ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
