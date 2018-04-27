using System;
using System.Reflection;
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
        #region SUBS/PUBS
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
        private readonly CompleteTaskPub _completeTaskPub = new CompleteTaskPub();
        readonly ResetSub _resetSub;
        readonly MechBoxSub _mechBoxSub;
        #endregion

        #region FLAGS
        private bool _started = false;
        private TankState _tankState;

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

        private bool _fusesOk;
        public bool FusesOk
        {
            get { return _fusesOk; }
            set
            {
                fusesLbl.Text = value ? "+" : "-";

                if (value)
                {
                    if (!FusesOk)
                        _audioMgr.PlayFusesOk();

                    _mainLightPub.LightOn();
                    _mainLightPub.LightOn();
                    _mainLightPub.LightOn();
                }
                else
                {
                    if (FusesOk)
                        _audioMgr.PlayPowerFail();

                    _mainLightPub.LightOff();
                    _mainLightPub.LightOff();
                    _mainLightPub.LightOff();
                }

                _fusesOk = value;
                UpdateElectricityAcriLight();
                UpdateCpuDoor();
            }
        }

        private bool _manipOk;
        public bool ManipOk
        {
            get { return _manipOk; }
            set
            {
                manipLbl.Text = value ? "+" : "-";

                if (value)
                {
                    if (!ManipOk)
                        _audioMgr.PlayManipOk();
                }

                _manipOk = value;

                UpdateElectricityAcriLight();
                UpdateCpuDoor();
            }
        }

        private bool _mechBoxOpen;
        public bool MechBoxOpen
        {
            get { return _mechBoxOpen; }
            set
            {
                meckBoxLbl.Text = value ? "+" : "-";
                _mechBoxOpen = value;
            }
        }

        private bool _sputnikLedOk;
        public bool SputnikLedOk
        {
            get { return _sputnikLedOk; }
            set
            {
                sputnikLedLbl.Text = value ? "+" : "-";
                _sputnikLedOk = value;
            }
        }

        private bool _gagOk;
        public bool GagOk
        {
            get { return _gagOk; }
            set
            {
                gagarinLbl.Text = value ? "+" : "-";

                if (value)
                {
                    _acriLightPub.SoundOn();

                    if (!GagOk)
                    {
                        _audioMgr.PlayVoiceOk();
                        _dVoiceOkEvent = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(6));
                    }
                }
                else
                {
                    _acriLightPub.SoundOff();

                    if (GagOk)
                        _audioMgr.PlayVoiceFail();
                }

                _gagOk = value;

                _creoDoorPub.GagRequest(value);
                UpdateCpuDoor();
            }
        }
        
        private bool _airOk;
        public bool AirOk
        {
            get { return _airOk; }
            set
            {
                airLbl.Text = value ? "+" : "-";

                if (value)
                {
                    if (!AirOk)
                        _audioMgr.PlayAirOk();
                    _acriLightPub.AirOn();
                }
                else
                {
                    if (AirOk)
                        _audioMgr.PlayAirFail();
                    _acriLightPub.AirOff();
                }

                _airOk = value;
                UpdateCpuDoor();
            }
        }

        private bool _sputnikOk;
        public bool SputnikOk
        {
            get { return _sputnikOk; }
            set
            {
                if (_started && !value)
                    return;

                spunikLbl.Text = value ? "+" : "-";

                if (value)
                {
                    _acriLightPub.SputnikOn();
                    _sputnikPub.SetOk();

                    if (!SputnikOk)
                        _audioMgr.PlaySputnikOk();
                }

                _sputnikOk = value;
            }
        }

        private bool _isDoorClosed;
        public bool IsDoorClosed
        {
            get { return _isDoorClosed; }
            set
            {
                creoDoorLbl.Text = value ? "+" : "-";
                _isDoorClosed = value;

                if (value)
                {
                    _creoDoorPub.CloseDoor();
                    doorBtn.Text = "Открыть дверь(крео)";
                }
                else
                {
                    _creoDoorPub.OpenDoor();
                    doorBtn.Text = "Закрыть дверь(крео)";
                }
            }
        }

        private bool _argonOk;
        public bool ArgonOk
        {
            get { return _argonOk; }
            set
            {
                argonLbl.Text = value ? "+" : "-";
                _argonOk = value;
            }
        }

        //private bool _airOk;
        //public bool AirOk
        //{
        //    get { return _airOk; }
        //    set
        //    {
        //        airLbl.Text = value ? "+" : "-";
        //        _airOk = value;
        //    }
        //}

        #endregion
        
        TimeSpan _gameDurationTimeSpan = TimeSpan.FromMinutes(63);
        readonly BackgroundAudio _backgroundAudioManager = new BackgroundAudio("Resources/Sounds/background.mp3");

        private TimeSpan _lastLightBlink = TimeSpan.MinValue;
        private TimeSpan _blinkDelay = TimeSpan.FromSeconds(40);
        private TimeSpan _dVoiceOkEvent = TimeSpan.MinValue;
        private TimeSpan _dMainOpenEvent = TimeSpan.MinValue;
        private TimeSpan _lastAudioEvent = TimeSpan.MaxValue;
        private TimeSpan _playSuccessMusic = TimeSpan.MinValue;

        private readonly TimeSpan GameLength = TimeSpan.FromMinutes(63);
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

            versionLbl.Text = Assembly.GetEntryAssembly().GetName().Version.ToString();
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
            _resetSub = new ResetSub(this);
            _mechBoxSub = new MechBoxSub(this);

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
            _completeTaskPub.Run();
            _resetSub.Run();
            _mechBoxSub.Run();

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

            Init();
        }

        private bool CriticalOk => FusesOk && ManipOk && GagOk && AirOk && SputnikOk;

        private void _audioMgr_OnPlayRequest()
        {
            _lastAudioEvent = _gameDurationTimeSpan;
        }

        private void UpdateCpuDoor()
        {
            if (CriticalOk)
            {
                _cpuDoorPub.OpenDoor();

                if (!_cpuOpen)
                {
                    _cpuOpen = true;
                    _audioMgr.PlayCpuOpen();
                }
            }
        }

        private void UpdateElectricityAcriLight()
        {
            if (FusesOk && ManipOk)
                _acriLightPub.LightOn();
            else
            {
                _acriLightPub.LightOff();
            }
        }

        public void SetBox(bool open, bool silent)
        {
            string text = open ? "Открыто" : "Закрыто";

            if (!silent)
                _audioMgr.PlayBoxClick();

            boxLbl.Text = text;
        }

        public void SetCreoDoor(bool ok, bool silent)
        {
            string text;

            if (ok)
            {
                text = "+";

                if (!silent)
                {
                    _audioMgr.PlayCreoDoorOpen();
                    _dMainOpenEvent = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(24));
                }
            }
            else
            {
                text = "-";
            }

            creoDoorLbl.Text = text;
        }

        public void TrySetSputnik(bool ok)
        {
            if (ok && !FusesOk)
            {
                SputnikOk = true;
            }
            else
            {
                SputnikOk = false;
            }
        }

        private void doorBtn_Click(object sender, System.EventArgs e)
        {
            var confirmResult = MessageBox.Show("Открыть/закрыть дверь?",
                         "Требуется подтверждение.",
                         MessageBoxButtons.OKCancel);
            if (confirmResult == DialogResult.OK)
            {
                IsDoorClosed = !IsDoorClosed;
            }
        }

        private void button1_Click(object sender, System.EventArgs e)
        {
            var confirmResult = MessageBox.Show(_started ? "Останавливаем игру?" : "Начинаем новую игру?",
                         "Текущее время будет сброшено.",
                         MessageBoxButtons.YesNo);

            if (confirmResult == DialogResult.Yes)
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
        }

        private void Init(bool saveTime = false)
        {
            _lastLightBlink = GameLength;
            _blinkDelay = StartBlinkDelay;

            tankLbl.Text = "Low";
            if (!saveTime)
                timerLbl.Text = "01:03:00";
            startButton.Text = "Старт";
            IsDoorClosed = true;
            _started = FusesOk = ManipOk = GagOk = AirOk = _cpuOpen = ArgonOk = SputnikOk = _dSputnik = SputnikLedOk = MechBoxOpen = false;
            _alert7 = _alert30 = _alert15 = _dVoiceOk = _dGameFail = _dMainOpen = _dAirAlert = _dElectroAlert = _successMusic = _alert10Engine = _alert40Energy = false;

            _resetPub.Ask();
            _dVoiceOkEvent = _dMainOpenEvent = _playSuccessMusic = TimeSpan.MinValue;
            _lastAudioEvent = TimeSpan.MaxValue;
            consoleTb.Clear();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            _gameDurationTimeSpan = _gameDurationTimeSpan.Subtract(TimeSpan.FromSeconds(1));
            timerLbl.Invoke(
                new Action(
                    () =>
                        timerLbl.Text =
                            $"{_gameDurationTimeSpan.Hours.ToString("00")}:{_gameDurationTimeSpan.Minutes.ToString("00")}:{_gameDurationTimeSpan.Seconds.ToString("00")}"));

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

                timer1.Stop(); //TODO: SHIT
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

            if (_gameDurationTimeSpan.Minutes == 30 && !_alert30 && !AirOk)
            {
                _alert30 = true;
                _audioMgr.D_Play30Alert();
            }

            if (_gameDurationTimeSpan.Minutes == 10 && !_alert10Engine && _cpuOpen)
            {
                _alert10Engine = true;
                _audioMgr.D_PlayEngineAlert10();
            }

            if (_gameDurationTimeSpan.Minutes == 40 && !_alert40Energy && !ManipOk)
            {
                _alert40Energy = true;
                _audioMgr.D_PlayEnergyAlert40();
            }

            if (_gameDurationTimeSpan.Minutes == 30 && !_alert30 && !AirOk)
            {
                _alert30 = true;
                _audioMgr.D_Play30Alert();
            }

            if ((_gameDurationTimeSpan.Minutes <= 45 && _gameDurationTimeSpan.Minutes >= 34 ||
                _gameDurationTimeSpan.Minutes <= 27 && _gameDurationTimeSpan.Minutes >= 19)
                && _lastAudioEvent - _gameDurationTimeSpan > TimeSpan.FromMinutes(4))
            {
                if (!_dElectroAlert && !ManipOk)
                {
                    _dElectroAlert = true;
                    _audioMgr.D_PlayEnergyAlert();
                }
                else if (!_dAirAlert && !AirOk)
                {
                    _dAirAlert = true;
                    _audioMgr.D_PlayAirAlert();
                }
                else if (!_dSputnik && !SputnikOk)
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

                if (FusesOk && !ManipOk)
                    _mainLightPub.LightBlink();
            }
        }

        private void button1_Click_1(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Открыть бак?",
                         "Требуется подтверждение.",
                         MessageBoxButtons.OKCancel);
            if (confirmResult == DialogResult.OK)
            {
                _tankPub.OpenTank();
            }
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

        private void initBtn_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Делаем инициализацию?",
                                     "Инициализация обнулит текущий прогресс игры.",
                                     MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _resetPub.Init();
                Init();

                _acriLightPub.AllOff();
                _gameDurationTimeSpan = GameLength;
                _resetPub.Ping();
            }
        }

        public void OpenTank()
        {
            _tankPub.OpenTank();
        }

        public void CloseTank()
        {
            _tankPub.CloseTank();
        }

        public void StartEngine(bool force = false)
        {
            if (force || 
                ((_tankState == TankState.Ok || _tankState == TankState.Overflow) &&
                AirOk && ArgonOk && ManipOk && GagOk && FusesOk && SputnikOk) )
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
            _creoDoorPub.GagRequest(GagOk);
        }

        private void QuestTest_FormClosing(object sender, FormClosingEventArgs e)
        {
            Log.Instance.Do("FORM CLOSING");
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
            _resetSub.Disconnect();
            _mechBoxSub.Disconnect();
            _completeTaskPub.Disconnect();
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

        public void WriteLineToConsole(string msg)
        {
            consoleTb.AppendText($"[{DateTime.Now}] : {msg}{Environment.NewLine}");
        }

        private void button9_Click(object sender, EventArgs e)
        {
            consoleTb.Clear();
        }

        private void button12_Click(object sender, EventArgs e)
        {
            _cpuPanelPub.HelpMap();
        }

        private void button13_Click(object sender, EventArgs e)
        {
        }

        private void button5_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Отметить пройденым Гагарина?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.GagarinOk();
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Отметить пройденым Креодверь?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.CreoDoorOk();
            }
        }

        private void button10_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Отметить пройденым генератор воздуха?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.AirOk();
            }
        }

        private void button11_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Отметить пройденым спутник?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.SputnikOk();
            }
        }

        private void button13_Click_1(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Отметить пройденым манипулятор?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.ManipOk();
            }
        }

        private void button14_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Установить значение 700 баку?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _completeTaskPub.Tank700();
            }
        }

        private void button15_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Открыть ЦПУ?", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                _cpuDoorPub.OpenDoor();
            }
        }

        private void button16_Click(object sender, EventArgs e)
        {
            var confirmResult = MessageBox.Show("Старт двигателя? Это успешно завершит игру", "Подтверждение", MessageBoxButtons.YesNo);
            if (confirmResult == DialogResult.Yes)
            {
                StartEngine(true);
            }
        }

        private void button17_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start(Log.Instance.Dir);
        }
    }


    public enum TankState
    {
        Low,
        Ok,
        Overflow
    }
}