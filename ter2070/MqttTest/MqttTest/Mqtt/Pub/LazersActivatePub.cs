using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class LazersActivatePub : MqttModule
    {
        public LazersActivatePub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Activate()
        {
            Publish("ter2070/tlazers/activate/device", "1");
        }

        public void Deactivate()
        {
            Publish("ter2070/tlazers/activate/device", "0"); 
        }
    }
}
