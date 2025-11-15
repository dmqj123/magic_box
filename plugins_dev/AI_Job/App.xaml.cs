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
                string aiCommand = string.Join(" ", e.Args);
                
                // 检查是否为"配置Ai"参数
                if (aiCommand == "配置Ai")
                {
                    // 显示API配置窗口
                    var configWindow = new ApiKeyConfigWindow();
                    configWindow.ShowDialog();
                    Shutdown();
                }
                else
                {
                    // 有其他参数，执行无界面模式
                    await ExecuteAiCommandSilent(aiCommand);
                    Shutdown(); // 执行完成后退出
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
