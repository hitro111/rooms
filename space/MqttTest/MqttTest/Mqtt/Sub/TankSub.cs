using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class TankSub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Ok = "1";
        private const string Over = "2";
        private const string Low = "0";

        public TankSub(QuestTest mainForm)
            : base("localhost")
        //: base("localhost")
        {
            _mainForm = mainForm;

            On["space/tank/out"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.TankVol(_.Message)));
                
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
