using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class ManipSub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Ok = "1";
        private const string No = "0";
        private const string OkSilent = "9";
        private const string NoSilent = "8";

        public ManipSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["space/manip/out"] = _ =>
            {
                if (_.Message == Ok)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.ManipOk = true));

                else if (_.Message == OkSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.ManipOk = true));

                else if (_.Message == NoSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.ManipOk = false));
            };
        }
    }
}
