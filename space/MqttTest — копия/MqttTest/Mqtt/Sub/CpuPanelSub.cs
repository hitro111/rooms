using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class CpuPanelSub : MqttModule
    {
        private readonly QuestTest _mainForm;

        public CpuPanelSub(QuestTest mainForm)
            //: base("192.168.0.59")
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/cpupanel/out"] = _ =>
            {
                if (_.Message == "0")
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.CloseTank()));
                else if (_.Message == "1")
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.OpenTank()));
                else if (_.Message == "2")
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.StartEngine()));
                else if (_.Message == "3")
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.Error()));
            };
        }
    }
}
