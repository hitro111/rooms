using Charlotte;

namespace MqttTest.Mqtt
{
    public class Commands : MqttModule
    {
        public Commands() : base("localhost") { }

        #region Color

        public void Color_Enable()
        {
            Publish("ter2070/tcolor/device", "e");
        }

        public void Color_Disable()
        {
            Publish("ter2070/tcolor/device", "d");
        }

        #endregion

        #region Door1

        public void Door1_Open()
        {
            Publish("ter2070/tdoor1/device", "o");
        }

        #endregion

        #region Door2

        public void Door2_AlarmOn()
        {

            Publish("ter2070/tdoor2/device", "a");
        }

        public void Door2_AlarmOff()
        {

            Publish("ter2070/tdoor2/device", "n");
        }

        public void Door2_OpenManually()
        {
            Publish("ter2070/tdoor2/device", "o");
        }

        #endregion

        #region Lazers

        public void Lazers_Activate()
        {
            Publish("ter2070/tlazers/activate/device", "1");
        }

        public void Lazers_Deactivate()
        {
            Publish("ter2070/tlazers/activate/device", "0");
        }

        public void Lazers_OnManual()
        {
            Publish("ter2070/tlazers/activate/device", "2");
        }

        public void Lazers_OffManual()
        {
            Publish("ter2070/tlazers/activate/device", "3");
        }

        public void Lazers_Init()
        {
            Publish("ter2070/tlazers/reset", "i");
        }

        public void Lazers_Ignore(int i)
        {
            Publish("ter2070/tlazers/ignore/device", (i - 1).ToString());
        }

        #endregion

        #region All

        public void All_Init()
        {
            Publish("ter2070/reset", "i");
        }

        public void All_Reset()
        {
            Publish("ter2070/reset", "r");
        }

        public void All_Ask()
        {
            Publish("ter2070/reset", "a");
        }

        public void All_Ping()
        {
            Publish("ter2070/ping/in", "p");
        }

        #endregion

        #region Second

        public void Second_Enable(bool quiet)
        {
            if (quiet)
            {
                Publish("ter2070/sec/in", "2");
            }
            else
            {
                Publish("ter2070/sec/in", "1");
            }
        }

        public void Second_Disable()
        {
            Publish("ter2070/sec/in", "0");
        }
        
        #endregion

        #region Smoke

        public void Smoke_On()
        {
            Publish("ter2070/c/smoke", "1");
        }

        public void Smoke_Off()
        {
            Publish("ter2070/c/smoke", "0");
        }

        #endregion
    }

    public static class Consts
    {
        public const char ServerId = '0';
        public const char Door1Id = '1';
    }
}
