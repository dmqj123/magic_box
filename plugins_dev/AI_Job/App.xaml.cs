using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using System.Configuration;
using System.Data;
using System.Windows;

namespace AI_Job
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        protected override async void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);
            
            // 检查是否有命令行参数
            if (e.Args.Length > 0)
            {
                string aiCommand = null;
                bool showConfig = false;
                
                // 解析命令行参数（参考AppSearch.cpp的参数传递方式）
                for (int i = 0; i < e.Args.Length; i++)
                {
                    string arg = e.Args[i];
                    
                    if (arg == "--config" || arg == "-c")
                    {
                        showConfig = true;
                    }
                    else if ((arg == "-k" || arg == "--keyword") && i + 1 < e.Args.Length)
                    {
                        aiCommand = e.Args[i + 1];
                        i++; // 跳过下一个参数，因为它已经被处理了
                    }
                    else if (aiCommand == null && !showConfig)
                    {
                        // 兼容旧的直接传递命令的方式
                        aiCommand = string.Join(" ", e.Args, i, e.Args.Length - i);
                        break;
                    }
                }
                
                if (showConfig)
                {
                    // 显示API配置窗口
                    var configWindow = new ApiKeyConfigWindow();
                    configWindow.ShowDialog();
                    Shutdown();
                }
                else if (aiCommand != null)
                {
                    // 执行无界面模式
                    await ExecuteAiCommandSilent(aiCommand);
                    Shutdown(); // 执行完成后退出
                }
                else
                {
                    // 无效参数，显示UI窗口
                    // StartupUri已经在App.xaml中设置为MainWindow.xaml
                }
            }
            else
            {
                // 无参数，显示UI窗口（保持原有行为）
                // StartupUri已经在App.xaml中设置为MainWindow.xaml
            }
        }
        
        private async Task ExecuteAiCommandSilent(string command)
        {
            Console.WriteLine("Ai操作中");
            
            try
            {
                // 执行AI命令（这里简化为直接返回结果）
                // 在实际应用中，这里会调用真正的AI处理逻辑
                await Task.Delay(1000); // 模拟处理时间
                Console.WriteLine("调用Ai完成");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"错误: {ex.Message}");
            }
        }
    }
}
