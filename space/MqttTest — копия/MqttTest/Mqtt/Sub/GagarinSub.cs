using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class GagarinSub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string GagOn = "1";
        private const string GagOff = "0";
        private const string GagOnSilent = "9";
        private const string GagOffSilent = "8";

        public GagarinSub(QuestTest mainForm)
            //: base("192.168.0.59")
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/gagarin/out"] = _ =>
            {
                if (_.Message == GagOn)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.SetGagarin(true, false)));

                else if (_.Message == GagOff)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.SetGagarin(false, false)));

                else if (_.Message == GagOnSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetGagarin(true, true)));

                else if (_.Message == GagOffSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetGagarin(false, true)));
            };
        }
    }
}
