using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class Box2SimpleSub : MqttModule
    {
        public Box2SimpleSub(QuestTest mainForm)
            : base("localhost")
        {
            On["ter2070/tboxsimple/out"] = _ =>
            {
                mainForm.Invoke((MethodInvoker)(mainForm.WriteSimpleBoxOk));
            };
        }
    }
}
