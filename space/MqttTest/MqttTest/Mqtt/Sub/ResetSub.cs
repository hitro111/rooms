using System.Net.NetworkInformation;
using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class ResetSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        public ResetSub(QuestTest mainForm)
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/ping/out"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.WriteLineToConsole("Ping received: " + _.Message)));
            };
        }
    }
}
