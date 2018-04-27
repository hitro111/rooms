using System;

namespace MqttTest.Audio.Space
{
    public class RoomAudioManager
    {
        private readonly SoundRecord _airFailRecord = new SoundRecord("Resources/Sounds/air_fail.wav", AudioEventCode.Air);
        private readonly SoundRecord _airOkRecord = new SoundRecord("Resources/Sounds/air_ok.wav", AudioEventCode.Air);
        private readonly SoundRecord _boxRecord = new SoundRecord("Resources/Sounds/box_click.wav", AudioEventCode.Box);
        private readonly SoundRecord _cpuOpenRecord = new SoundRecord("Resources/Sounds/cpu_open.wav", AudioEventCode.CpuOpen);
        private readonly SoundRecord _creoDoorRecord = new SoundRecord("Resources/Sounds/creodoor_ok.wav", AudioEventCode.CreoDoor);
        private readonly SoundRecord _engineErrRecord = new SoundRecord("Resources/Sounds/engine_error.wav", AudioEventCode.Engine);
        private readonly SoundRecord _engineStartRecord = new SoundRecord("Resources/Sounds/engine_start.wav", AudioEventCode.Engine);
        private readonly SoundRecord _gameFailRecord = new SoundRecord("Resources/Sounds/fail.mp3", AudioEventCode.GameFail);
        private readonly SoundRecord _powerFailRecord = new SoundRecord("Resources/Sounds/power_fail.wav", AudioEventCode.Power);
        private readonly SoundRecord _powerOkFusesRecord = new SoundRecord("Resources/Sounds/power_ok_fuses.wav", AudioEventCode.Power);
        private readonly SoundRecord _powerOkManipRecord = new SoundRecord("Resources/Sounds/power_ok_manipulator.wav", AudioEventCode.Power);
        private readonly SoundRecord _sputnikOkRecord = new SoundRecord("Resources/Sounds/sputnik_ok.wav", AudioEventCode.Sputnik);
        private readonly SoundRecord _gameSuccessRecord = new SoundRecord("Resources/Sounds/success.mp3", AudioEventCode.GameSuccess);
        private readonly SoundRecord _voiceOkRecord = new SoundRecord("Resources/Sounds/voice_ok.wav", AudioEventCode.Voice);
        private readonly SoundRecord _voiceFailRecord = new SoundRecord("Resources/Sounds/voice_fail.wav", AudioEventCode.Voice);

        private readonly SoundRecord _dAlert15 = new SoundRecord("Resources/Sounds/d/d_15_alert.wav", volume: 0.6f);
        private readonly SoundRecord _dAlert7 = new SoundRecord("Resources/Sounds/d/d_7_alert.wav", volume: 0.6f);
        private readonly SoundRecord _dAlert30 = new SoundRecord("Resources/Sounds/d/d_air_30_alert.wav", volume: 0.6f);
        private readonly SoundRecord _dAlertAir = new SoundRecord("Resources/Sounds/d/d_air_alert.wav", volume: 0.6f);
        private readonly SoundRecord _dAlertEnergy = new SoundRecord("Resources/Sounds/d/d_energy_alert.wav", volume: 0.6f);
        private readonly SoundRecord _dGameFail = new SoundRecord("Resources/Sounds/d/d_game_fail.wav", volume: 0.6f);
        private readonly SoundRecord _dMainOpen = new SoundRecord("Resources/Sounds/d/d_main_open.wav", volume: 0.6f);
        private readonly SoundRecord _dVoiceOk = new SoundRecord("Resources/Sounds/d/d_voice_ok.wav", volume: 0.6f);
        private readonly SoundRecord _dAlertEnergy40 = new SoundRecord("Resources/Sounds/d/d_energy_40.wav", volume: 0.6f);
        private readonly SoundRecord _dAlertEngine10 = new SoundRecord("Resources/Sounds/d/d_engine_10.wav", volume: 0.6f);
        private readonly SoundRecord _dSputnik = new SoundRecord("Resources/Sounds/d/d_sputnik.wav", volume: 0.6f);
        private readonly SoundRecord _dSuccess = new SoundRecord("Resources/Sounds/d/d_success.wav", volume: 1.0f);

        private readonly AudioDeviceMgr _creoAudio;
        private readonly AudioDeviceMgr _mainAudio;
        private readonly AudioDeviceMgr _cpuAudio;

        public bool GameRunning { get; set; }

        public event Action OnPlayRequest;

        public RoomAudioManager(AudioDeviceMgr creoMgr,
            AudioDeviceMgr mainMgr,
            AudioDeviceMgr cpuMgr)
        {
            _creoAudio = creoMgr;
            _mainAudio = mainMgr;
            _cpuAudio = cpuMgr;
        }

        public void ChangeCreoChannel(int deviceNum)
        {
            _creoAudio.ChangeChannel(deviceNum);
        }

        public void ChangeMainChannel(int deviceNum)
        {
            _mainAudio.ChangeChannel(deviceNum);
        }

        public void ChangeCpuChannel(int deviceNum)
        {
            _cpuAudio.ChangeChannel(deviceNum);
        }

        private void PlayQueued(AudioDeviceMgr mgr, IPlayItem playItem)
        {
            if (GameRunning)
            {
                mgr.PlayQueued(playItem);
                OnPlayRequest?.Invoke();
            }
        }



        public void PlayAirFail()
        {
            PlayQueued(_mainAudio, _airFailRecord);
        }

        public void PlayAirOk()
        {
            PlayQueued(_mainAudio, _airOkRecord);
        }

        public void PlayCpuOpen()
        {
            PlayQueued(_mainAudio, _cpuOpenRecord);
        }

        public void PlayCreoDoorOpen()
        {
            PlayQueued(_creoAudio, _creoDoorRecord);
            OnPlayRequest?.Invoke();
        }

        public void PlayEngineError()
        {
            PlayQueued(_cpuAudio, _engineErrRecord);
        }

        public void PlayEngineStart()
        {
            PlayQueued(_cpuAudio, _engineStartRecord);
        }

        public void PlayGameFail()
        {
            _mainAudio.PlayNow(_gameFailRecord);
            _creoAudio.PlayNow(_gameFailRecord);
            _cpuAudio.PlayNow(_gameFailRecord);
            OnPlayRequest?.Invoke();
        }

        public void PlayPowerFail()
        {
            PlayQueued(_mainAudio, _powerFailRecord);
        }

        public void PlayFusesOk()
        {
            PlayQueued(_creoAudio, _powerOkFusesRecord);
        }

        public void PlayManipOk()
        {
            PlayQueued(_mainAudio, _powerOkManipRecord);
        }

        public void PlaySputnikOk()
        {
            PlayQueued(_mainAudio, _sputnikOkRecord);
        }

        public void PlayBoxClick()
        {
            PlayQueued(_creoAudio, _boxRecord);
        }

        public void D_PlaySuccess()
        {
            _mainAudio.PlayQueued(_dSuccess);
        }

        public void D_PlaySputnik()
        {
            _mainAudio.PlayQueued(_dSputnik);
        }

        public void D_PlayEnergyAlert40()
        {
            _mainAudio.PlayQueued(_dAlertEnergy40);
        }

        public void D_PlayEngineAlert10()
        {
            _mainAudio.PlayQueued(_dAlertEngine10);
        }

        public void PlaySuccessMusic()
        {
            _mainAudio.PlayNow(_gameSuccessRecord);
            _creoAudio.PlayNow(_gameSuccessRecord);
            OnPlayRequest?.Invoke();
        }

        public void PlayVoiceFail()
        {
            PlayQueued(_creoAudio, _voiceFailRecord);
        }

        public void PlayVoiceOk()
        {
            PlayQueued(_creoAudio, _voiceOkRecord);
        }

        public void D_Play15Alert()
        {
            PlayQueued(_mainAudio, _dAlert15);
        }

        public void D_Play30Alert()
        {
            PlayQueued(_mainAudio, _dAlert30);
        }

        public void D_Play7Alert()
        {
            PlayQueued(_mainAudio, _dAlert7);
        }

        public void D_PlayAirAlert()
        {
            PlayQueued(_mainAudio, _dAlertAir);
            OnPlayRequest?.Invoke();
        }

        public void D_PlayEnergyAlert()
        {
            PlayQueued(_mainAudio, _dAlertEnergy);
        }

        public void D_PlayGameFail()
        {
            PlayQueued(_mainAudio, _dGameFail);
        }

        public void D_PlayMainOpen()
        {
            PlayQueued(_mainAudio, _dMainOpen);
        }

        public void D_PlayVoiceOk()
        {
            PlayQueued(_creoAudio, _dVoiceOk);
        }

        public void TestCreoDevice()
        {
            _creoAudio.PlayNow(_boxRecord);
        }

        public void TestMainDevice()
        {
            _mainAudio.PlayNow(_boxRecord);
        }

        public void TestCpuDevice()
        {
            _cpuAudio.PlayNow(_boxRecord);
        }
    }
}
