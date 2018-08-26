using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class ColorPub : MqttModule
    {
        public ColorPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Enable()
        {
            Publish("ter2070/tcolor/device", "e");
        }

        public void Disable()
        {
            Publish("ter2070/tcolor/device", "d");
        }
    }
}
