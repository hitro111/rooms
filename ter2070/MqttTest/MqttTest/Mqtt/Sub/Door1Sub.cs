using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class Door1Sub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Ok = "1";

        public Door1Sub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["ter2070/tdoor1/server"] = _ =>
            {

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Door1Enabled()));
                
                //if (_.Message == Ok)
                //    _mainForm.Invoke((MethodInvoker) (() => _mainForm.SetTank(TankState.Ok)));

                //else if (_.Message == Over)
                //    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetTank(TankState.Overflow)));

                //else if (_.Message == Low)
                //    _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetTank(TankState.Low)));
            };
        }
    }
}
