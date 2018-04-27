using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class SputnikSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string Finished = "1";
        private const string FinishedSilent = "9";
        private const string RunningSilent = "8";

        public SputnikSub(QuestTest mainForm)
            //: base("192.168.0.59")
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/sputnik/out"] = _ =>
            {
                if (_.Message == Finished)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(true, false)));

                else if (_.Message == FinishedSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(true, true)));

                else if (_.Message == RunningSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(false, false)));
            };
        }
    }
}
