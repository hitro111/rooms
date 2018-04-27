using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class SputnikPub : MqttModule
    {
        public SputnikPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void SetOk()
        {
            Publish("space/sputnik/in", "b");
        }
    }
}
