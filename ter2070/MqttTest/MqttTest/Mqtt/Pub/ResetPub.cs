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
            Publish("ter2070/reset", "i");
        }

        public void Reset()
        {
            Publish("ter2070/reset", "r");
        }

        public void Ask()
        {
            Publish("ter2070/reset", "a");
        }

        public void Ping()
        {
            Publish("ter2070/ping/in", "p");
        }
    }
}
