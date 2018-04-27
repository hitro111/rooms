using System;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using MqttTest.Audio;
using MqttTest.Audio.Space;
using MqttTest.Mqtt.Pub;
using MqttTest.Mqtt.Sub;

namespace MqttTest
{
    public partial class QuestTest : Form
    {
        readonly CreoDoorPub _creoDoorPub = new CreoDoorPub();
        readonly TankPub _tankPub = new TankPub();
        readonly TankSub _tankSub;
        readonly FusesSub _fusesSub;
        readonly GagarinSub _gagarinSub;
        readonly SputnikSub _sputnikSub;
        readonly CreoDoorSub _creoDoorSub;
        readonly CreoBoxSub _creoBoxSub;
        readonly AcriLightPub _acriLightPub = new AcriLightPub();
        readonly AirSub _airSub;
        readonly ManipSub _manipSub;
        private readonly CpuDoorPub _cpuDoorPub = new CpuDoorPub();
        readonly CpuPanelSub _cpuPanelSub;
        readonly ResetPub _resetPub = new ResetPub();
        readonly CpuPanelPub _cpuPanelPub = new CpuPanelPub();
        readonly MainLightPub _mainLightPub = new MainLightPub();
        readonly ArgonBoxSub _argonBoxSub;
        readonly ConsoleSub _consoleSub;
        private readonly SputnikPub _sputnikPub = new SputnikPub();

        TimeSpan _gameDurationTimeSpan = new TimeSpan(1, 0, 0);
        readonly BackgroundAudio _backgroundAudioManager = new BackgroundAudio("Resources/Sounds/background.mp3");

        private bool _fusesOk = false;
        private bool _manipOk = false;
        private bool _gagOk = false;
        private bool _airOk = false;
        private bool _started = false;
        private bool _sputnikOk = false;
        private bool _manualSputnikOk = false;
        private bool _isDoorClosed;
        private TankState _tankState;
        private bool _argonOk;

        private bool _alert7;
        private bool _alert15;
        private bool _alert30;
        private bool _alert40Energy;
        private bool _alert10Engine;

        private bool _dVoiceOk;
        private bool _dMainOpen;
        private bool _dGameFail;
        private bool _dAirAlert;
        private bool _dElectroAlert;
        private bool _successMusic;
        private bool _dSputnik;

        private bool _cpuOpen;

        private TimeSpan _lastLightBlink = TimeSpan.MinValue;
        private TimeSpan _blinkDelay = TimeSpan.FromSeconds(40);

        private TimeSpan _dVoiceOkEvent = TimeSpan.MinValue;
        private TimeSpan _dMainOpenEvent = TimeSpan.MinValue;
        private TimeSpan _lastAudioEvent = TimeSpan.MaxValue;

        private TimeSpan _playSuccessMusic = TimeSpan.MinValue;

        private readonly TimeSpan GameLength = TimeSpan.FromHours(1);
        private readonly TimeSpan StartBlinkDelay = TimeSpan.FromSeconds(80);
        private readonly TimeSpan BlinkDelayDec = TimeSpan.FromSeconds(2);
        private readonly TimeSpan MinBlinkDelay = TimeSpan.FromSeconds(20);

        private const int DefaultCreoChannel = 1;
        private const int DefaultMainChannel = 3;
        private const int DefaultCpuChannel = 2;

        AudioDeviceMgr creoMgr = new AudioDeviceMgr(new AudioDevice(DefaultCreoChannel));
        AudioDeviceMgr mainMgr = new AudioDeviceMgr(new AudioDevice(DefaultMainChannel));
        AudioDeviceMgr cpuMgr = new AudioDeviceMgr(new AudioDevice(DefaultCpuChannel));
        private readonly RoomAudioManager _audioMgr;

        public QuestTest()
        {
            InitializeComponent();

            creoUpDown.SelectedIndex = DefaultCreoChannel;
            mainUpDown.SelectedIndex = DefaultMainChannel;
            cpuUpDown.SelectedIndex = DefaultCpuChannel;

            _fusesSub = new FusesSub(this);
            _gagarinSub = new GagarinSub(this);
            _sputnikSub = new SputnikSub(this);
            _creoDoorSub = new CreoDoorSub(this);
            _creoBoxSub = new CreoBoxSub(this);
            _airSub = new AirSub(this);
            _tankSub = new TankSub(this);
            _manipSub = new ManipSub(this);
            _cpuPanelSub = new CpuPanelSub(this);
            _argonBoxSub = new ArgonBoxSub(this);
            _consoleSub = new ConsoleSub(this);

            _fusesSub.Run();
            _gagarinSub.Run();
            _sputnikSub.Run();
            _creoDoorPub.Run();
            _resetPub.Run();
            _creoDoorSub.Run();
            _creoBoxSub.Run();
            _acriLightPub.Run();
            _airSub.Run();
            _tankPub.Run();
            _manipSub.Run();
            _tankSub.Run();
            _cpuDoorPub.Run();
            _cpuPanelSub.Run();
            _cpuPanelPub.Run();
            _mainLightPub.Run();
            _argonBoxSub.Run();
            _consoleSub.Run();
            _sputnikPub.Run();

            Init();

            _audioMgr = new RoomAudioManager(creoMgr, mainMgr, cpuMgr);
            new Thread(() =>
            {
                Thread.CurrentThread.IsBackground = true;
                creoMgr.Run();
            }).Start();
            new Thread(() =>
            {
                Thread.CurrentThread.IsBackground = true;
                mainMgr.Run();
            }).Start();
            new Thread(() =>
            {
                Thread.CurrentThread.IsBackground = true;
                cpuMgr.Run();
            }).Start();
            _audioMgr.OnPlayRequest += _audioMgr_OnPlayRequest;
        }

        private void _audioMgr_OnPlayRequest()
        {
            _lastAudioEvent = _gameDurationTimeSpan;
        }

        private bool CriticalOk => _fusesOk && _manipOk && _gagOk && _airOk && (_sputnikOk || _manualSputnikOk);

        private void UpdateCpuDoor(bool silent)
        {
            if (CriticalOk)
            {
                _cpuDoorPub.OpenDoor();

                if (!silent && !_cpuOpen)
                {
                    _cpuOpen = true;
                    _audioMgr.PlayCpuOpen();
                }
            }
        }

        private void UpdateElectricityAcriLight()
        {
            if (_fusesOk && _manipOk)
                _acriLightPub.LightOn();
            else
            {
                _acriLightPub.LightOff();
            }
        }

        public void SetFuses(bool on, bool silent)
        {
            string text = on ? "Ок" : "Нет";

            if (!silent)
            {
                if (on)
                {
                    if (!_fusesOk)
                        _audioMgr.PlayFusesOk();
                }
                else
                {
                    if (_fusesOk)
                        _audioMgr.PlayPowerFail();
                }
            }

            _fusesOk = on;

            if (_fusesOk)
            {
                _mainLightPub.LightOn();
                _mainLightPub.LightOn();
                _mainLightPub.LightOn();
            }
            else
            {
                _mainLightPub.LightOff();
                _mainLightPub.LightOff();
                _mainLightPub.LightOff();
            }

            UpdateElectricityAcriLight();

            fusesLbl.Text = text;
            UpdateCpuDoor(silent);
        }

        public void SetAir(bool on, bool silent)
        {
            string text = on ? "Ок" : "Нет";

            if (on)
            {
                if (!silent && !_airOk)
                    _audioMgr.PlayAirOk();
                _acriLightPub.AirOn();
            }
            else
            {
                if (!silent && _airOk)
                    _audioMgr.PlayAirFail();
                _acriLightPub.AirOff();
            }

            _airOk = on;


            UpdateCpuDoor(silent);


            airLbl.Text = text;
        }

        public void SetBox(bool open, bool silent)
        {
            string text = open ? "Открыто" : "Нет";

            if (!silent)
                _audioMgr.PlayBoxClick();

            boxLbl.Text = text;
        }

        public void SetGagarin(bool ok, bool silent)
        {
            string text;

            if (ok)
            {
                text = "OK";
                _acriLightPub.SoundOn();

                if (!silent && !_gagOk)
                {
                    _audioMgr.PlayVoiceOk();
                    _dVoiceOkEvent = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(6));
                }
            }
            else
            {
                text = "Нет";
                _acriLightPub.SoundOff();

                if (!silent && _gagOk)
                    _audioMgr.PlayVoiceFail();
            }

            _gagOk = ok;

            _creoDoorPub.GagRequest(ok);
            UpdateCpuDoor(silent);

            gagarinLbl.Text = text;
        }

        public void SetCreoDoor(bool ok, bool silent)
        {
            string text;

            if (ok)
            {
                text = "OK";

                if (!silent)
                {
                    _audioMgr.PlayCreoDoorOpen();
                    _dMainOpenEvent = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(24));
                }

                //dp.OpenDoor();
            }
            else
            {
                text = "Нет";
            }

            creoDoorLbl.Text = text;
        }

        public void TrySetSputnik(bool ok, bool silent)
        {
            string text;

            if (ok && !_fusesOk)
            {
                text = "OK";
                if (!_sputnikOk)
                {
                    _sputnikOk = true;
                    _acriLightPub.SputnikOn();
                    _sputnikPub.SetOk();

                    if (!silent)
                        _audioMgr.PlaySputnikOk();
                }
            }
            else
            {
                text = "Нет";
                _sputnikOk = false;
            }

            spunikLbl.Text = text;
        }

        private void doorBtn_Click(object sender, System.EventArgs e)
        {
            _isDoorClosed = !_isDoorClosed;
            if (_isDoorClosed)
            {
                _creoDoorPub.CloseDoor();
                doorBtn.Text = "Открыть";
            }
            else
            {
                _creoDoorPub.OpenDoor();
                doorBtn.Text = "Закрыть";
            }
        }

        private void button1_Click(object sender, System.EventArgs e)
        {
            if (!_started)
            {
                _audioMgr.GameRunning = true;
                startButton.Text = "Стоп";
                _gameDurationTimeSpan = GameLength;
                timer1.Start();
                _started = true;

                _backgroundAudioManager.PlayBackground();
            }
            else
            {
                _audioMgr.GameRunning = false;
                Init();
                _acriLightPub.AllOn();
                timer1.Stop();
                _backgroundAudioManager.StopAll();
            }
        }

        private void Init(bool saveTime = false)
        {
            _lastLightBlink = GameLength;
            _blinkDelay = StartBlinkDelay;
            gagarinLbl.Text =
                fusesLbl.Text = spunikLbl.Text = creoDoorLbl.Text = boxLbl.Text = airLbl.Text = manipLbl.Text = "Нет";
            tankLbl.Text = "Low";
            if (!saveTime)
                timerLbl.Text = "60:00";
            startButton.Text = "Старт";
            _started = _fusesOk = _manipOk = _gagOk = _airOk = _isDoorClosed = _cpuOpen = _argonOk = _sputnikOk = _manualSputnikOk = _dSputnik = false;
            _alert7 = _alert30 = _alert15 = _dVoiceOk = _dGameFail = _dMainOpen = _dAirAlert = _dElectroAlert = _successMusic = _alert10Engine = _alert40Energy = false;
            button11.Text = "NO";
            _resetPub.Ask();
            _dVoiceOkEvent = _dMainOpenEvent = _playSuccessMusic = TimeSpan.MinValue;
            _lastAudioEvent = TimeSpan.MaxValue;
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            _gameDurationTimeSpan = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(1));
            timerLbl.Invoke(
                new Action(
                    () =>
                        timerLbl.Text =
                            $"{_gameDurationTimeSpan.Minutes.ToString("00")}:{_gameDurationTimeSpan.Seconds.ToString("00")}"));

            ProcessLightBlink();

            ProcessTimeAlerts();

            ProcessAudioEvents();

            if (_gameDurationTimeSpan <= TimeSpan.FromSeconds(20) && !_dGameFail)
            {
                _dGameFail = true;
                _audioMgr.D_PlayGameFail();
            }

            if (_gameDurationTimeSpan <= TimeSpan.Zero)
            {
                timer1.Stop();
                _backgroundAudioManager.StopAll();
                _audioMgr.PlayGameFail();
                _audioMgr.GameRunning = false;
                Init(true);
            }
        }

        private void ProcessAudioEvents()
        {
            if (_gameDurationTimeSpan <= _dVoiceOkEvent && !_dVoiceOk)
            {
                _dVoiceOk = true;
                _audioMgr.D_PlayVoiceOk();
            }
            
            if (_gameDurationTimeSpan <= _dMainOpenEvent && !_dMainOpen)
            {
                _dMainOpen = true;
                _audioMgr.D_PlayMainOpen();
            }

            if (_gameDurationTimeSpan <= _playSuccessMusic && !_successMusic)
            {
                _successMusic = true;
                _audioMgr.PlaySuccessMusic();
            }
        }

        private void ProcessTimeAlerts()
        {
            if (_gameDurationTimeSpan.Minutes == 7 && !_alert7)
            {
                _alert7 = true;
                _audioMgr.D_Play7Alert();
            }

            if (_gameDurationTimeSpan.Minutes == 15 && !_alert15)
            {
                _alert15 = true;
                _audioMgr.D_Play15Alert();
            }

            if (_gameDurationTimeSpan.Minutes == 30 && !_alert30 && !_airOk)
            {
                _alert30 = true;
                _audioMgr.D_Play30Alert();
            }

            if (_gameDurationTimeSpan.Minutes == 10 && !_alert10Engine && _cpuOpen)
            {
                _alert10Engine = true;
                _audioMgr.D_PlayEngineAlert10();
            }

            if (_gameDurationTimeSpan.Minutes == 40 && !_alert40Energy && !_manipOk)
            {
                _alert40Energy = true;
                _audioMgr.D_PlayEnergyAlert40();
            }

            if (_gameDurationTimeSpan.Minutes == 30 && !_alert30 && !_airOk)
            {
                _alert30 = true;
                _audioMgr.D_Play30Alert();
            }

            if ((_gameDurationTimeSpan.Minutes <= 45 && _gameDurationTimeSpan.Minutes >= 34 ||
                _gameDurationTimeSpan.Minutes <= 27 && _gameDurationTimeSpan.Minutes >= 19)
                && _lastAudioEvent - _gameDurationTimeSpan > TimeSpan.FromMinutes(4))
            {
                if (!_dElectroAlert && !_manipOk)
                {
                    _dElectroAlert = true;
                    _audioMgr.D_PlayEnergyAlert();
                }
                else if (!_dAirAlert && !_airOk)
                {
                    _dAirAlert = true;
                    _audioMgr.D_PlayAirAlert();
                }
                else if (!_dSputnik && !_sputnikOk)
                {
                    _dSputnik = true;
                    _audioMgr.D_PlaySputnik();
                }
            }
        }

        private void ProcessLightBlink()
        {
            if (_lastLightBlink - _gameDurationTimeSpan > _blinkDelay)
            {
                if (_blinkDelay > MinBlinkDelay)
                    _blinkDelay = _blinkDelay.Subtract(BlinkDelayDec);

                _lastLightBlink = _gameDurationTimeSpan;

                if (_fusesOk && !_manipOk)
                    _mainLightPub.LightBlink();
            }
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            _tankPub.OpenTank();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            _audioMgr.TestCreoDevice();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            _audioMgr.TestMainDevice();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            _audioMgr.TestCpuDevice();
        }

        private void domainUpDown1_SelectedItemChanged(object sender, EventArgs e)
        {
            new Thread(() =>
            {
                _audioMgr?.ChangeCreoChannel(((DomainUpDown)sender).SelectedIndex);
            }).Start();
            
        }

        private void tabPage3_Click(object sender, EventArgs e)
        {

        }

        private void domainUpDown3_SelectedItemChanged(object sender, EventArgs e)
        {
            new Thread(() =>
            {
                _audioMgr?.ChangeCpuChannel(((DomainUpDown)sender).SelectedIndex);
            }).Start();
        }

        private void domainUpDown2_SelectedItemChanged(object sender, EventArgs e)
        {
            new Thread(() =>
            {
                _audioMgr?.ChangeMainChannel(((DomainUpDown)sender).SelectedIndex);
            }).Start();
        }

        public void SetManip(bool b, bool silent)
        {
            manipLbl.Text = b ? "OK" : "Нет";


            if (!silent && !_manipOk)
                _audioMgr.PlayManipOk();

            _manipOk = b;

            UpdateElectricityAcriLight();

            UpdateCpuDoor(silent);
        }

        private void initBtn_Click(object sender, EventArgs e)
        {
            Init();
            _resetPub.Init();
            Thread.Sleep(1000);
            _acriLightPub.AllOff();
            _gameDurationTimeSpan = GameLength;
        }

        public void OpenTank()
        {
            _tankPub.OpenTank();
        }

        public void CloseTank()
        {
            _tankPub.CloseTank();
        }

        public void StartEngine()
        {
            if ((_tankState == TankState.Ok || _tankState == TankState.Overflow) &&
                _airOk && _argonOk && _manipOk && _gagOk && _fusesOk)
            {
                _acriLightPub.EngineOk();
                _cpuPanelPub.Success();
                _backgroundAudioManager.StopAll();
                _audioMgr.PlayEngineStart();

                _audioMgr.D_PlaySuccess();
                _playSuccessMusic = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(12));
            }
            else
            {
                _cpuPanelPub.Error();
            }
        }

        public void Error()
        {
            _audioMgr.PlayEngineError();
        }

        public void GagRequest()
        {
            _creoDoorPub.GagRequest(_gagOk);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //mgr.PlayQueued("Sounds/box_click.wav");
        }

        private void button6_Click(object sender, EventArgs e)
        {
            //mgr.PlayNow("Sounds/box_click.wav");
        }

        private void QuestTest_FormClosing(object sender, FormClosingEventArgs e)
        {
            timer1.Dispose();
            _backgroundAudioManager.DisposeAll();
            _fusesSub.Disconnect();
            _gagarinSub.Disconnect();
            _sputnikSub.Disconnect();
            _creoDoorPub.Disconnect();
            _resetPub.Disconnect();
            _creoDoorSub.Disconnect();
            _creoBoxSub.Disconnect();
            _acriLightPub.Disconnect();
            _airSub.Disconnect();
            _tankPub.Disconnect();
            _manipSub.Disconnect();
            _tankSub.Disconnect();
            _cpuDoorPub.Disconnect();
            _cpuPanelSub.Disconnect();
            _cpuPanelPub.Disconnect();
            _mainLightPub.Disconnect();
            _argonBoxSub.Disconnect();
            _consoleSub.Disconnect();
            _sputnikPub.Disconnect();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            _mainLightPub.LightOn();
        }

        private void button8_Click(object sender, EventArgs e)
        {
            _mainLightPub.LightOff();
        }

        private void tabPage1_Click(object sender, EventArgs e)
        {

        }

        public void TankVol(string message)
        {
            try
            {
                tankLbl.Text = message;
                int val = Convert.ToInt32(message);
                if (val < 300)
                    _tankState = TankState.Low;
                else if (val >= 300 & val <= 700)
                    _tankState = TankState.Ok;
                else
                    _tankState = TankState.Overflow;

                if (_tankState == TankState.Ok || _tankState == TankState.Overflow)
                {
                    _acriLightPub.FuelOn();
                }
            }
            catch (Exception)
            {
                
                //
            }
        }

        public void SetArgon(bool ok)
        {
            _argonOk = true;
        }

        public void WriteLineToConsole(string msg)
        {
            var sb = new StringBuilder(consoleTb.Text);

            sb.AppendLine($"[{DateTime.Now}] : {msg}");

            consoleTb.Text = sb.ToString();
        }

        private void button9_Click(object sender, EventArgs e)
        {
            consoleTb.Text = string.Empty;
        }

        private void button10_Click(object sender, EventArgs e)
        {
            _creoDoorPub.OpenDoor();
        }

        private void button11_Click(object sender, EventArgs e)
        {
            _manualSputnikOk = true;
            button11.Text = "OK";
        }

        private void button12_Click(object sender, EventArgs e)
        {
            _cpuPanelPub.HelpMap();
        }

        private void button13_Click(object sender, EventArgs e)
        {
        }
    }


    public enum TankState
    {
        Low,
        Ok,
        Overflow
    }
}