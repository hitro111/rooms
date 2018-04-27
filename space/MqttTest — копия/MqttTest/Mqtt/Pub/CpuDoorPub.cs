using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class CpuDoorPub : MqttModule
    {
        public CpuDoorPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void OpenDoor()
        {
            Publish("space/cpudoor/in", "1");
        }
    }
}
