using Charlotte;

namespace MqttTest.Mqtt.Pub
{
    public class AcriLightPub : MqttModule
    {
        public const string _EngineOn = "e1";
        public const string _LightOn = "l1";
        public const string _LightOff = "l0";
        public const string _AirOn = "a1";
        public const string _AirOff = "a0";
        public const string _SoundOn = "s1";
        public const string _SoundOff = "s0";
        public const string _FuelOn = "f1";
        public const string _FuelOff = "f0";
        public const string _SputnikOn = "k1";
        public const string _AllOff = "z0";
        public const string _AllOn = "z1";

        public AcriLightPub()
            //: base("192.168.0.59")
            : base("localhost")
        {
        }

        public void EngineOk()
        {
            Publish("space/acrilight/in", _EngineOn);
        }

        public void LightOn()
        {
            Publish("space/acrilight/in", _LightOn);
        }

        public void LightOff()
        {
            Publish("space/acrilight/in", _LightOff);
        }

        public void AirOn()
        {
            Publish("space/acrilight/in", _AirOn);
        }

        public void AirOff()
        {
            Publish("space/acrilight/in", _AirOff);
        }

        public void SoundOn()
        {
            Publish("space/acrilight/in", _SoundOn);
        }

        public void SoundOff()
        {
            Publish("space/acrilight/in", _SoundOff);
        }

        public void FuelOn()
        {
            Publish("space/acrilight/in", _FuelOn);
        }

        public void FuelOff()
        {
            Publish("space/acrilight/in", _FuelOff);
        }

        public void SputnikOn()
        {
            Publish("space/acrilight/in", _SputnikOn);
        }

        public void AllOff()
        {
            Publish("space/acrilight/in", _AllOff);
        }

        public void AllOn()
        {
            Publish("space/acrilight/in", _AllOn);
        }
    }
}
