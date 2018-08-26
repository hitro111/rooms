using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class LazersIgnoreSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        public LazersIgnoreSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["ter2070/tlazers/ignore/server"] = _ =>
            {
                int lazer = int.Parse(_.Message);
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetIgnore(lazer + 1)));
            };
        }
    }
}
