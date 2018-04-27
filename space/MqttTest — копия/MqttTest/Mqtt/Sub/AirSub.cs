using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class AirSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string AirOk = "1";
        private const string AirOff = "0";
        private const string AirOkSilent = "9";
        private const string AirOffSilent = "8";

        public AirSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["space/air/out"] = _ =>
            {
                if (_.Message == AirOk)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.SetAir(true, false)));

                else if (_.Message == AirOff)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetAir(false, false)));

                else if (_.Message == AirOkSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetAir(true, true)));

                else if (_.Message == AirOffSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetAir(false, true)));
            };
        }
    }
}
