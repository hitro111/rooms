using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class Door2Pub : MqttModule
    {
        public Door2Pub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void AlarmOn()
        {

            Publish("ter2070/tdoor2/device", "a");
        }

        public void AlarmOff()
        {

            Publish("ter2070/tdoor2/device", "n");
        }

        public void OpenManually()
        {
            Publish("ter2070/tdoor2/device", "o");
        }

        public void MockOpenDevice()
        {
            Publish("ter2070/tdoor2/server", "1");
        }
    }
}
