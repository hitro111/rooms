using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt.Sub
{
    public class MechBoxSub : MqttModule
    {
        private readonly QuestTest _mainForm;
        private const string Open = "1";
        private const string Closed = "0";

        public MechBoxSub(QuestTest mainForm)
            : base("localhost")
        {
            _mainForm = mainForm;

            On["space/mechbox/out"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => mainForm.MechBoxOpen = _.Message == Open));
                
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
