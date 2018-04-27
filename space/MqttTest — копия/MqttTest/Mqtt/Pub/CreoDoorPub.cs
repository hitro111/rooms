using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class CreoDoorPub : MqttModule
    {
        private const string GagOn = "4";
        private const string GagOff = "5";

        public CreoDoorPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void OpenDoor()
        {
            Publish("space/creodoor/in", "1");
        }

        public void CloseDoor()
        {
            Publish("space/creodoor/in", "0");
        }

        public void GagRequest(bool gagOk)
        {
            Publish("space/creodoor/in", gagOk ? GagOn : GagOff);
        }
    }
}
