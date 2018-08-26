using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class LazersAlarmSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        public LazersAlarmSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["ter2070/tlazers/alert/server"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.LazersAlert()));
            };
        }
    }
}
