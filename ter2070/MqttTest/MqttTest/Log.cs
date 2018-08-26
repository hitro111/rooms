using System;
using System.IO;
using System.Text;
using System.Windows.Forms.VisualStyles;

namespace MqttTest
{
    public class Log
    {
        private readonly string _filePath;
        private readonly string _dir;
        private static Log _instance;

        private Log()
        {
            if (!Directory.Exists($"{Directory.GetCurrentDirectory()}\\logs"))
            {
                Directory.CreateDirectory($"{Directory.GetCurrentDirectory()}\\logs");
            }

            _dir = $"{Directory.GetCurrentDirectory()}\\logs";

            _filePath = $"{Directory.GetCurrentDirectory()}\\logs\\log_{DateTime.Now.Day}_{DateTime.Now.Month}_{DateTime.Now.Year}.txt";
        }

        public string FilePath => _filePath;
        public string Dir => _dir;

        public void Do(string info)
        {
            var startLine = $"{Environment.NewLine}---------------- INFO {DateTime.Now} ---------------- {Environment.NewLine}";
            var endLine = $"{Environment.NewLine}---------------- END INFO ---------------- {Environment.NewLine}";
            File.AppendAllText(_filePath, $"{startLine}{info}{endLine}");
        }

        public void Do(Exception ex)
        {
            StringBuilder sb = new StringBuilder();
            var startLine = $"---------------- ERROR {DateTime.Now} ----------------";
            var endLine = $"---------------- END ERROR ----------------{Environment.NewLine}";

            sb.AppendLine(startLine);
            sb.AppendLine($"EXECPTION:   {ex.Message}\n{ex.StackTrace}\n");
            int i = 0;
            var cur = ex;
            while (cur.InnerException != null)
            {
                ++i;
                cur = cur.InnerException;
                sb.AppendLine($"INNER {i}:   {cur.Message}{Environment.NewLine}{cur.StackTrace}{Environment.NewLine}");
            }
            sb.AppendLine(endLine);

            File.AppendAllText(_filePath, $"{sb}");
        }

        public static Log Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new Log();
                }
                return _instance;
            }
        }
    }
}
