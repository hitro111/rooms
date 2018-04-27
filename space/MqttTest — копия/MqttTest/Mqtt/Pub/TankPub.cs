using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class TankPub : MqttModule
    {
        public TankPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void OpenTank()
        {
            Publish("space/tank/in", "1");
        }

        public void CloseTank()
        {
            Publish("space/tank/in", "0");
        }
    }
}
