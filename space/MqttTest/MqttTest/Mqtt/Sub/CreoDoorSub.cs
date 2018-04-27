using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class CreoDoorSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        private const string DoorOpen = "1";
        private const string DoorClosed = "0";
        private const string DoorOpenSilent = "9";
        private const string DoorClosedSilent = "8";

        private const string GagRequest = "3";

        public CreoDoorSub(QuestTest mainForm)
            //: base("192.168.0.59")
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/creodoor/out"] = _ =>
            {
                if (_.Message == DoorOpen)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetCreoDoor(true, false)));

                else if (_.Message == DoorClosed)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetCreoDoor(false, false)));

                else if (_.Message == DoorOpenSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetCreoDoor(true, true)));

                else if (_.Message == DoorClosedSilent)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetCreoDoor(false, true)));

                else if (_.Message == GagRequest)
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.GagRequest()));
            };
        }
    }
}
