using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class FusesSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string FusesOk = "1";
        private const string FusesFail = "0";
        private const string FusesOkSilent = "9";
        private const string FusesFailSilent = "8";

        public FusesSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["space/fuses/out"] = _ =>
            {
                if (_.Message == FusesOk)
                    _mainForm.Invoke((MethodInvoker) (() => _mainForm.FusesOk = true));

                else if (_.Message == FusesFail)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.FusesOk = false));

                else if (_.Message == FusesOkSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.FusesOk = true));

                else if (_.Message == FusesFailSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.FusesOk = false));
            };
        }
    }
}
