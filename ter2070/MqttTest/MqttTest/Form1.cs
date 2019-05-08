using System;
using System.Globalization;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using MqttTest.Audio;
using MqttTest.Mqtt;
using Timer = System.Timers.Timer;

namespace MqttTest
{
    public partial class QuestTest : Form
    {
        private const int DefaultChannel = 0;
        private readonly Commands _commands = new Commands();
        private Events _events;
        readonly BackgroundAudio _backgroundAudio = new BackgroundAudio("Resources/Sounds/background.mp3", DefaultChannel);
        AudioDeviceMgr _deviceAudioMgr = new AudioDeviceMgr(new AudioDevice(DefaultChannel));
        private RoomAudioManager _audioMgr;
        private Timer _timer;
        private DateTime _startTime;

        private int secGameCount = 0;

        public QuestTest()
        {
            InitializeComponent();

            _audioMgr = new RoomAudioManager(_deviceAudioMgr);
            new Thread(() =>
            {
                Thread.CurrentThread.IsBackground = true;
                _deviceAudioMgr.Run();
            }).Start();

            Task.Run(() =>
            {
                Task.Delay(2000);
                InitMqtt();
                Init();
            });
        }

        private void InitMqtt()
        {
            _events = new Events(this);

            //_commands.Run();
            //_events.Run();
        }

        private void Init()
        {
            secGameCount = 0;

            //_commands.All_Init();
            //_commands.All_Ping();
            consoleTb.Clear();

            //_commands.Color_Enable();
            //_commands.Lazers_Init();

            //Thread.Sleep(500); 
            //_events.InitPower();
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
            _audioMgr?.PlaySmokeOn();
            _commands.Smoke_On();
        }

        private void button18_Click(object sender, EventArgs e)
        {
            secLbl.Text = "<>";
            _commands.Second_Enable(false);
        }

        public void WriteBlinkBoxOk()
        {
            blinkBoxLbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
        }

        public void LazersAlert()
        {
            _audioMgr?.PlayAlarmOn();
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
            _audioMgr.GameRunning = true;
            _backgroundAudio.ChangeTrack("Resources/Sounds/background.mp3");

            try
            {
                if (_timer != null)
                {
                    _timer.Stop();
                    _timer.Dispose();
                }
            }
            catch
            {
                //
            }

            _startTime = DateTime.Now;
            _timer = new Timer();
            _timer.Interval = 60000;

            _timer.Elapsed += OnTimedEvent;
            _timer.AutoReset = true;
            _timer.Enabled = true;

        }

        private static void OnTimedEvent(Object source, System.Timers.ElapsedEventArgs e)
        {
            //todo timer sounds
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

            _commands.Second_Enable(secGameCount > 0);
            secGameCount++;

            _audioMgr?.PlayAlarmActive();
            _backgroundAudio.ChangeTrack("Resources/Sounds/mission_impossible.mp3");
        }

        public void Door2Opened()
        {
            _commands.Second_Disable();
            _commands.Lazers_Deactivate();
            door2Lbl.Text = DateTime.Now.ToString(CultureInfo.InvariantCulture);
            _audioMgr?.PlayLabOpen();
            _backgroundAudio.ChangeTrack("Resources/Sounds/background.mp3");
        }

        public void Tumbler(int value)
        {
            tumblerLbl.Text = Convert.ToString(value, 2);
            if (value == 31438)
            {
                _audioMgr?.PlayGeneratorReady();
            }
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

        public void Sound(int value)
        {
            soundPwrLbl.Text = value.ToString();
        }

        public void Tv(int value)
        {
            tvPwrLbl.Text = value.ToString();
        }

        private void smokeBtn_Click(object sender, EventArgs e)
        {
            if (smokeBtn.Text == "Smoke ON")
            {
                smokeBtn.Text = "Smoke OFF";
                _commands.Smoke_On();
            }
            else
            {
                smokeBtn.Text = "Smoke ON";
                _commands.Smoke_Off();
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            _commands.Lazers_OnManual();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            _commands.Lazers_OffManual();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            _backgroundAudio.Stop();
            _audioMgr.GameRunning = false;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            _backgroundAudio.ChangeTrack("Resources/Sounds/mission_impossible.mp3");
        }
    }
}