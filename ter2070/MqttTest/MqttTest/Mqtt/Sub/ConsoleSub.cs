using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class ConsoleSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        public ConsoleSub(QuestTest mainForm)
            : base("localhost")
        {
            _mainForm = mainForm;

            On["ter2070/console/device"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.WriteLineToConsole(_.Message)));
            };
        }
    }
}
