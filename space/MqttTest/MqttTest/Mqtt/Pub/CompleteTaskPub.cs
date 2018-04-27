using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class CompleteTaskPub : MqttModule
    {
        public CompleteTaskPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void GagarinOk()
        {
            Publish("space/gagarin/out", "1");
            Publish("space/gagarin/in", "1");
        }

        public void CreoDoorOk()
        {
            Publish("space/creodoor/out", "1");
        }

        public void AirOk()
        {
            Publish("space/air/out", "1");
            Publish("space/air/in", "o");
        }

        public void SputnikOk()
        {
            Publish("space/sputnik/out", "7");
        }

        public void ManipOk()
        {
            Publish("space/manip/out", "1");
        }

        public void Tank700()
        {
            Publish("space/tank/out", "700");
        }
    }
}
