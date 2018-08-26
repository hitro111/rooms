using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class Door1Pub : MqttModule
    {
        public Door1Pub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void OpenManually()
        {
            Publish("ter2070/tdoor1/device", "o");
        }

        public void MockOpenDevice()
        {
            Publish("ter2070/tdoor1/server", "1");
        }
    }
}
