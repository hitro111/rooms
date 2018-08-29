using System;
using System.Globalization;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Flurl.Http;
using MqttTest.Mqtt.Pub;
using MqttTest.Mqtt.Sub;

namespace MqttTest
{
    public partial class QuestTest : Form
    {
        #region SUBS/PUBS

        private readonly AllSub _allSub;

        readonly ConsoleSub _consoleSub;
        readonly ResetSub _resetSub;
        private readonly ResetPub _resetPub = new ResetPub();

        readonly SecPub _secPub = new SecPub();
        readonly SecSub _secSub;

        readonly LazersInitPub _lazersInitPub = new LazersInitPub();
        readonly LazersActivatePub _lazersActivatePub = new LazersActivatePub();
        readonly LazersIgnorePub _lazersIgnorePub = new LazersIgnorePub();
        readonly LazersAlarmSub _lazersAlarmSub;
        readonly LazersIgnoreSub _lazersIgnoreSub;

        readonly Box1BlinkSub _boxBlinkSub;

        readonly Door2Pub _door2Pub = new Door2Pub();
        private readonly Door2Sub _door2Sub;
        readonly Door1Pub _door1Pub = new Door1Pub();
        private readonly Door1Sub _door1Sub;

        readonly ColorPub _colorPub = new ColorPub();

        private readonly ColorSub _colorSub;
        #endregion

        #region FLAGS

        #endregion

        bool[] _ignoredLazers = new bool[6];

        public QuestTest()
        {
            var res = "http://testingpol.azurewebsites.net/Home/Add".PostJsonAsync(new
            {
                Topic = "App",
                Payload = "Form created"
            }).Result;

            InitializeComponent();

            _allSub = new AllSub(this);

            _consoleSub = new ConsoleSub(this);
            _resetSub = new ResetSub(this);
            _secSub = new SecSub(this);
            _boxBlinkSub = new Box1BlinkSub(this);
            _lazersIgnoreSub = new LazersIgnoreSub(this);
            _lazersAlarmSub = new LazersAlarmSub(this);
            _door2Sub = new Door2Sub(this);
            _colorSub = new ColorSub(this);
            _door1Sub = new Door1Sub(this);

            _allSub.Run();

            _consoleSub.Run();
            _resetSub.Run();

            _secSub.Run();
            _secPub.Run();
            _boxBlinkSub.Run();

            _lazersAlarmSub.Run();
            _lazersIgnoreSub.Run();
            _lazersActivatePub.Run();
            _lazersInitPub.Run();
            _lazersIgnorePub.Run();

            _door2Sub.Run();
            _door2Pub.Run();
            _door1Sub.Run();
            _door1Pub.Run();
            _colorSub.Run();
            _colorPub.Run();

            Init();
        }


        private void Init()
        {
            _resetPub.Init();
            _resetPub.Ping();
            consoleTb.Clear();
        }

        private void Start()
        {
            
        }

        private void QuestTest_FormClosing(object sender, FormClosingEventArgs e)
        {
            Log.Instance.Do("FORM CLOSING");
            _allSub.Disconnect();
            _consoleSub.Disconnect();
            _resetSub.Disconnect();
            _secPub.Disconnect();
            _boxBlinkSub.Disconnect();
            _lazersAlarmSub.Disconnect();
            _lazersIgnoreSub.Disconnect();
            _lazersActivatePub.Disconnect();
            _lazersInitPub.Disconnect();
            _lazersIgnorePub.Disconnect();
            _door2Sub.Disconnect();
            _door2Pub.Disconnect();
            _resetPub.Disconnect();
            _colorPub.Disconnect();
            _colorSub.Disconnect();
            _door1Sub.Disconnect();
            _door1Pub.Disconnect();
        }
        
        public void WriteLineToConsole(string msg)
        {
            consoleTb.AppendText($"[{DateTime.Now}] : {msg}{Environment.NewLine}");
        }

        private void button9_Click(object sender, EventArgs e)
        {
            consoleTb.Clear();
        }

        public void SetSec()
        {
            secLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        private void button18_Click(object sender, EventArgs e)
        {
            secLbl.Text = "<>";
            _secPub.Enable();
        }

        public void WriteBlinkBoxOk()
        {
            blinkBoxLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        public void LazersAlert()
        {
            alarmLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
            _door2Pub.AlarmOn();
            _colorPub.Enable();
        }

        public void WriteSimpleBoxOk()
        {
            simpleBoxLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        public void SetIgnore(int i)
        {
            switch (i)
            {
                case 1:
                    laz1Btn.Text = "1 X";
                    break;
                case 2:
                    laz2Btn.Text = "2 X";
                    break;
                case 3:
                    laz3Btn.Text = "3 X";
                    break;
                case 4:
                    laz4Btn.Text = "4 X";
                    break;
                case 5:
                    laz5Btn.Text = "5 X";
                    break;
                case 6:
                    laz6Btn.Text = "6 X";
                    break;
            }
        }

        private void initLazersBtn_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < 6; ++i)
            {
                _ignoredLazers[i] = false;
            }

            laz1Btn.Text = "1 ON";
            laz2Btn.Text = "2 ON";
            laz3Btn.Text = "3 ON";
            laz4Btn.Text = "4 ON";
            laz5Btn.Text = "5 ON";
            laz6Btn.Text = "6 ON";

            _lazersInitPub.Init();
        }

        private void activateLazersBtn_Click(object sender, EventArgs e)
        {
            _lazersActivatePub.Activate();
        }

        private void laz1Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(1);
        }

        private void laz2Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(2);
        }

        private void laz3Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(3);
        }

        private void laz4Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(4);
        }

        private void laz5Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(5);
        }

        private void laz6Btn_Click(object sender, EventArgs e)
        {
            _lazersIgnorePub.Ignore(6);
        }

        private void startBtn_Click(object sender, EventArgs e)
        {
            Init();
        }

        private void door1Open_Click(object sender, EventArgs e)
        {
            _door1Pub.MockOpenDevice();
        }

        public void Door1Enabled()
        {
            _colorPub.Enable();
        }

        public void ColorOk()
        {
            _door2Pub.AlarmOff();
            _lazersActivatePub.Activate();
            _colorPub.Disable();
            _secPub.Enable();
            //TODO: open door 1
        }

        public void Door2Opened()
        {
            _lazersActivatePub.Deactivate();
            door2Lbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }
    }
}