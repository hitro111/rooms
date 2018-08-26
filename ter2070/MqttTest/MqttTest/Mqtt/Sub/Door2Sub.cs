using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class Door2Sub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Ok = "1";

        public Door2Sub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["ter2070/tdoor2/server"] = _ =>
            {

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Door2Opened()));

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
