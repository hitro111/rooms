using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class ArgonBoxSub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Ok = "1";
        private const string No = "0";

        public ArgonBoxSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["space/argonbox/out"] = _ =>
            {
                if (_.Message == Ok)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.SetArgon(false)));

                else if (_.Message == No)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetArgon(true)));
            };
        }
    }
}
