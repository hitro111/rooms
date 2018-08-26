using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class Box1BlinkSub : MqttModule
    {
        public Box1BlinkSub(QuestTest mainForm)
            : base("localhost")
        {
            On["ter2070/tboxblink/out"] = _ =>
            {
                mainForm.Invoke((MethodInvoker)(mainForm.WriteBlinkBoxOk));
            };
        }
    }
}
