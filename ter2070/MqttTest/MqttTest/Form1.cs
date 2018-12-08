using System;
using System.Globalization;
using System.Threading;
using System.Windows.Forms;
using MqttTest.Mqtt;

namespace MqttTest
{
    public partial class QuestTest : Form
    {
        private readonly Commands _commands = new Commands();
        private readonly Events _events;

        public QuestTest()
        {
            InitializeComponent();

            _events = new Events(this);

            _commands.Run();
            _events.Run();

            Init();
        }


        private void Init()
        {
            _commands.All_Init();
            _commands.All_Ping();
            consoleTb.Clear();

            _commands.Color_Enable();
            _commands.Lazers_Init();

            Thread.Sleep(500);
            _events.InitPower();
        }

        private void Start()
        {
            
        }

        private void QuestTest_FormClosing(object sender, FormClosingEventArgs e)
        {
            Log.Instance.Do("FORM CLOSING");
            _commands.Disconnect();
            _events.Disconnect();
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
            _commands.Second_Enable();
        }

        public void WriteBlinkBoxOk()
        {
            blinkBoxLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        public void LazersAlert()
        {
            alarmLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
            _commands.Door2_AlarmOn();
            _commands.Color_Enable();

            Thread.Sleep(100);

            _commands.Color_Enable();
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
            laz1Btn.Text = "1 ON";
            laz2Btn.Text = "2 ON";
            laz3Btn.Text = "3 ON";
            laz4Btn.Text = "4 ON";
            laz5Btn.Text = "5 ON";
            laz6Btn.Text = "6 ON";

            _commands.Lazers_Init();
        }

        private void activateLazersBtn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Activate();
        }

        private void laz1Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(1);
        }

        private void laz2Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(2);
        }

        private void laz3Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(3);
        }

        private void laz4Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(4);
        }

        private void laz5Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(5);
        }

        private void laz6Btn_Click(object sender, EventArgs e)
        {
            _commands.Lazers_Ignore(6);
        }

        private void startBtn_Click(object sender, EventArgs e)
        {
            Init();
        }

        private void door1Open_Click(object sender, EventArgs e)
        {
            _commands.Door1_Open();
        }

        public void Door1Enabled()
        {
            _commands.Color_Enable();
        }

        public void ColorOk()
        {
            _commands.Door2_AlarmOff();
            _commands.Door1_Open();
            _commands.Lazers_Activate();
            _commands.Color_Disable();
            _commands.Second_Enable();
        }

        public void Door2Opened()
        {
            _commands.Lazers_Deactivate();
            door2Lbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        public void Tumbler(int value)
        {
            tumblerLbl.Text = Convert.ToString(value, 2);
        }

        public void Door1(int value)
        {
            door1PwrLbl.Text = value.ToString();
        }

        public void Block1(int value)
        {
            block1PwrLbl.Text = value.ToString();
        }

        public void Block2(int value)
        {
            block2PwrLbl.Text = value.ToString();
        }

        public void Generator(int value)
        {
            genPwrLbl.Text = value.ToString();
        }
    }
}