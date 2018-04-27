using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class ResetPub : MqttModule
    {
        public ResetPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Init()
        {
            Publish("space/reset", "i");
        }

        public void Reset()
        {
            Publish("space/reset", "r");
        }

        public void Ask()
        {
            Publish("space/reset", "a");
        }
    }
}
