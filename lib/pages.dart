import 'package:flutter/material.dart';

class SettingsPage extends StatefulWidget {
  const SettingsPage({super.key});

  @override
  State<SettingsPage> createState() => _SettingsPageState();
}

class _SettingsPageState extends State<SettingsPage> {
  int _selectedIndex = 0;

  final List<Widget> _pages = <Widget>[
    const Center(child: Text('通用设置内容')), // General Settings
    const Center(child: Text('账户设置内容')), // Account Settings
    const Center(child: Text('隐私设置内容')), // Privacy Settings
    const Center(child: Text('关于内容')), // About
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('设置'),
      ),
      body: Row(
        children: <Widget>[
          // Left Menu Bar
          NavigationRail(
            selectedIndex: _selectedIndex,
            onDestinationSelected: (int index) {
              setState(() {
                _selectedIndex = index;
              });
            },
            labelType: NavigationRailLabelType.all,
            destinations: const <NavigationRailDestination>[
              NavigationRailDestination(
                icon: Icon(Icons.settings),
                label: Text('通用'),
              ),
              NavigationRailDestination(
                icon: Icon(Icons.person),
                label: Text('账户'),
              ),
              NavigationRailDestination(
                icon: Icon(Icons.lock),
                label: Text('隐私'),
              ),
              NavigationRailDestination(
                icon: Icon(Icons.info),
                label: Text('关于'),
              ),
            ],
          ),
          const VerticalDivider(thickness: 1, width: 1),
          // Right Page Body
          Expanded(
            child: _pages[_selectedIndex],
          ),
        ],
      ),
    );
  }
}