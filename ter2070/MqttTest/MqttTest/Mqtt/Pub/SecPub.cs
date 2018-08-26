using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class SecPub : MqttModule
    {
        public SecPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Enable()
        {
            Publish("ter2070/sec/in", "1");
        }
    }
}
