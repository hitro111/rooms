using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class MainLightPub : MqttModule
    {
        public MainLightPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void LightOn()
        {
            Publish("space/mlight/in", "1");
        }

        public void LightOff()
        {
            Publish("space/mlight/in", "0");
        }

        public void LightBlink()
        {
            Publish("space/mlight/in", "3");
        }
    }
}
