using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class SputnikSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string Finished = "1";
        private const string Running = "0";
        private const string ForceFromApp = "7";
        private const string FinishedSilent = "9";
        private const string RunningSilent = "8";
        private const string LedOn = "d";

        public SputnikSub(QuestTest mainForm)
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/sputnik/out"] = _ =>
            {
                if (_.Message == Finished)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(true)));

                else if (_.Message == Running)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(false)));

                else if (_.Message == FinishedSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(true)));

                else if (_.Message == RunningSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.TrySetSputnik(false)));

                else if (_.Message == ForceFromApp)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SputnikOk = true));

                else if (_.Message == LedOn)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SputnikLedOk = true));
            };
        }
    }
}
