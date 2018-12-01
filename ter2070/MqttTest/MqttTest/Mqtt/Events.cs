using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Charlotte;

namespace MqttTest.Mqtt
{
    class Events : MqttModule
    {
        private List<string> ignored = new List<string>()
        {
            "^space" //ignore second room
        };
        private readonly QuestTest _mainForm;

        public Events(QuestTest mainForm) : base("localhost")
        {
            _mainForm = mainForm;

            #region Power changed events

            On["ter2070/e/door1Pwr"] = _ =>
            {
                var senderPowerTuple = ((string) _.Message).DeserializePowerData();
                Console.WriteLine($"sender: {senderPowerTuple.Item1}, value: {senderPowerTuple.Item2}");
                if (senderPowerTuple.Item1 == Consts.ServerId)
                    return;

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Door1(senderPowerTuple.Item2)));
            };

            On["ter2070/e/block1Pwr"] = _ =>
            {
                var senderPowerTuple = ((string)_.Message).DeserializePowerData();
                Console.WriteLine($"sender: {senderPowerTuple.Item1}, value: {senderPowerTuple.Item2}");
                if (senderPowerTuple.Item1 == Consts.ServerId)
                    return;

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Block1(senderPowerTuple.Item2)));
            };

            On["ter2070/e/block2Pwr"] = _ =>
            {
                var senderPowerTuple = ((string)_.Message).DeserializePowerData();
                Console.WriteLine($"sender: {senderPowerTuple.Item1}, value: {senderPowerTuple.Item2}");
                if (senderPowerTuple.Item1 == Consts.ServerId)
                    return;

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Block1(senderPowerTuple.Item2)));
            };

            On["ter2070/e/genPwr"] = _ =>
            {
                var senderPowerTuple = ((string)_.Message).DeserializePowerData();
                Console.WriteLine($"sender: {senderPowerTuple.Item1}, value: {senderPowerTuple.Item2}");
                if (senderPowerTuple.Item1 == Consts.ServerId)
                    return;

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Generator(senderPowerTuple.Item2)));
            };

            #endregion

            #region TODO:revise

            On["#"] = _ =>
            {
                if (!ignored.Contains(_.Topic) && !ignored.Any(x => Regex.IsMatch(_.Topic, x)))
                {
                    Console.WriteLine($"[{DateTime.Now}]{_.Topic}: {_.Message}");
                }
            };

            On["ter2070/tboxsimple/out"] = _ =>
            {
                mainForm.Invoke((MethodInvoker)(mainForm.WriteSimpleBoxOk));
            };

            On["ter2070/tboxblink/out"] = _ =>
            {
                mainForm.Invoke((MethodInvoker)(mainForm.WriteBlinkBoxOk));
            };

            On["ter2070/tcolor/server"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.ColorOk()));
            };

            On["ter2070/logs/server"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.WriteLineToConsole(_.Message)));
            };

            On["ter2070/tdoor1/server"] = _ =>
            {
                string msg = _.Message;
                if (msg == "1")
                {
                    _mainForm.Invoke((MethodInvoker)(() => _mainForm.Door1Enabled()));
                }
            };

            On["ter2070/tdoor2/server"] = _ =>
            {

                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Door2Opened()));
            };

            On["ter2070/tlazers/alert/server"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.LazersAlert()));
            };

            On["ter2070/tlazers/ignore/server"] = _ =>
            {
                int lazer = int.Parse(_.Message);
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetIgnore(lazer + 1)));
            };

            On["space/ping/out"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.WriteLineToConsole("Ping received: " + _.Message)));
            };

            On["ter2070/sec/out"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.SetSec()));
            };

            On["ter2070/ttumblr/server"] = _ =>
            {
                _mainForm.Invoke((MethodInvoker)(() => _mainForm.Tumbler(int.Parse(_.Message))));
            };

            #endregion
        }

        public void InitPower()
        {
            Door1Power(167);
            Block1Power(180);
            Block2Power(112);
            GeneratorPower(66);
        }

        public void Door1Power(int power)
        {
            Publish("ter2070/e/door1Pwr", Ext.SerializePowerData(Consts.ServerId, power));
        }

        public void Block1Power(int power)
        {
            Publish("ter2070/e/block1Pwr", Ext.SerializePowerData(Consts.ServerId, power));
        }

        public void Block2Power(int power)
        {
            Publish("ter2070/e/block2Pwr", Ext.SerializePowerData(Consts.ServerId, power));
        }

        public void GeneratorPower(int power)
        {
            Publish("ter2070/e/genPwr", Ext.SerializePowerData(Consts.ServerId, power));
        }
    }

    public static partial class Ext
    {
        public static string SerializePowerData(char senderId, int power)
        {
            return senderId + power.ToString();
        }

        public static Tuple<char, int> DeserializePowerData(this string data)
        {
            char senderId = data.First();
            int value = int.Parse(data.Substring(1));

            return new Tuple<char, int>(senderId, value);
        }
    }
}
