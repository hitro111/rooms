using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class LazersIgnorePub : MqttModule
    {
        public LazersIgnorePub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Ignore(int i)
        {
            Publish("ter2070/tlazers/ignore/device", (i-1).ToString());
        }
    }
}
