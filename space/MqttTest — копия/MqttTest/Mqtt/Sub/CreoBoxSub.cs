using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class CreoBoxSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string BoxOpen = "1";
        private const string BoxClosed = "0";
        private const string BoxOpenSilent = "9";
        private const string BoxClosedSilent = "8";

        public CreoBoxSub(QuestTest mainForm)
            //: base("192.168.0.59")
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/creobox/out"] = _ =>
            {
                if (_.Message == BoxOpen)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetBox(true, false)));

                else if (_.Message == BoxClosed)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetBox(false, false)));

                else if (_.Message == BoxOpenSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetBox(true, true)));

                else if (_.Message == BoxClosedSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetBox(false, true)));
            };
        }
    }
}
