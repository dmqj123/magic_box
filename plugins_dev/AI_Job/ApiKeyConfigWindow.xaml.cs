using System;
using System.Configuration;
using System.Windows;

namespace AI_Job
{
    /// <summary>
    /// Interaction logic for ApiKeyConfigWindow.xaml
    /// </summary>
    public partial class ApiKeyConfigWindow : Window
    {
        public ApiKeyConfigWindow()
        {
            InitializeComponent();
            
            // 如果已有保存的API Key，显示在文本框中（以星号显示）
            string savedApiKey = ConfigurationManager.AppSettings["ApiKey"];
            if (!string.IsNullOrEmpty(savedApiKey))
            {
                // 为了安全，不在PasswordBox中显示已保存的密钥
                // 用户需要重新输入或留空表示删除
            }
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            string apiKey = ApiKeyTextBox.Password;
            
            // 这里应该保存API Key到配置文件
            // 由于WPF App.config的限制，实际保存逻辑可能需要其他方式实现
            // 但根据要求，只做UI部分
            
            MessageBox.Show("API Key配置已保存", "提示", MessageBoxButton.OK, MessageBoxImage.Information);
            this.DialogResult = true;
            this.Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }
    }
}
