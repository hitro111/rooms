using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class CpuPanelPub : MqttModule
    {
        public const string ArgonOkCmd = "g";
        public const string ArgonFailCmd = "f";
        public const string HelpMapCmd = "h";

        public CpuPanelPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void Error()
        {
            Publish("space/cpupanel/in", "e");
        }

        public void Success()
        {
            Publish("space/cpupanel/in", "z");
        }

        public void ArgonOk()
        {
            Publish("space/cpupanel/in", ArgonOkCmd);
        }

        public void ArgonFail()
        {
            Publish("space/cpupanel/in", ArgonFailCmd);
        }

        public void HelpMap()
        {
            Publish("space/cpupanel/in", HelpMapCmd);
        }
    }
}
