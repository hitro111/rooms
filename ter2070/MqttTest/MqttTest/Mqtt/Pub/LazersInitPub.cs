using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class LazersInitPub : MqttModule
    {
        public LazersInitPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Init()
        {
            Publish("ter2070/tlazers/reset", "i");
        }
    }
}
