using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace AI_Job
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private async void ExecuteButton_Click(object sender, RoutedEventArgs e)
        {
            string userInput = InputTextBox.Text.Trim();
            if (string.IsNullOrEmpty(userInput))
            {
                MessageBox.Show("请输入指令", "提示", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            // 显示"Ai操作中"
            AiStatusText.Text = "Ai操作中";
            
            try
            {
                string commandResult = await ExecuteSystemCommandAsync(userInput);
                AiStatusText.Text = "";
                ShowResultWindow(commandResult ?? "操作完成");
            }
            catch (Exception ex)
            {
                AiStatusText.Text = "";
                MessageBox.Show($"执行出错: {ex.Message}", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private async Task<string> ExecuteSystemCommandAsync(string userInput)
        {
            await Task.Delay(1000);
            return "调用Ai完成";
        }

        private void ShowResultWindow(string result)
        {
            // 创建结果显示窗口
            Window resultWindow = new Window
            {
                Title = "AI操作结果",
                Width = 600,
                Height = 400,
                WindowStartupLocation = WindowStartupLocation.CenterOwner
            };

            TextBox resultTextBox = new TextBox
            {
                Text = result,
                AcceptsReturn = true,
                TextWrapping = TextWrapping.Wrap,
                IsReadOnly = true,
                FontFamily = new System.Windows.Media.FontFamily("Consolas"),
                FontSize = 12,
                Margin = new Thickness(10)
            };

            resultWindow.Content = resultTextBox;
            resultWindow.Owner = this;
            resultWindow.ShowDialog();
        }
    }
}
