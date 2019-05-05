using System;

namespace MqttTest.Audio
{
    public class RoomAudioManager
    {
        private readonly SoundRecord _alarmActiveRecord = new SoundRecord("Resources/Sounds/alarm_activated.wav", AudioEventCode.Alarm);
        private readonly SoundRecord _alarmOnRecord = new SoundRecord("Resources/Sounds/alarm_on.wav", AudioEventCode.Alarm);
        private readonly SoundRecord _labOpenRecord = new SoundRecord("Resources/Sounds/lab_open.wav", AudioEventCode.LabDoor);
        private readonly SoundRecord _generatorReadyRecord = new SoundRecord("Resources/Sounds/generator_ready.wav", AudioEventCode.Generator);
        private readonly SoundRecord _gunReadyRecord = new SoundRecord("Resources/Sounds/gun_ready.wav", AudioEventCode.Gun);
        private readonly SoundRecord _gunShotBgRecord = new SoundRecord("Resources/Sounds/gun_shotbg.wav", AudioEventCode.Gun);


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

        private readonly AudioDeviceMgr _deviceMgr;

        public bool GameRunning { get; set; }

        public event Action OnPlayRequest;

        public RoomAudioManager(AudioDeviceMgr deviceMgr)
        {
            _deviceMgr = deviceMgr;
        }

        public void ChangeChannel(int deviceNum)
        {
            _deviceMgr.ChangeChannel(deviceNum);
        }

        private void PlayQueued(IPlayItem playItem)
        {
            if (GameRunning)
            {
                try
                {
                    _deviceMgr.PlayQueued(playItem);
                    OnPlayRequest?.Invoke();
                }
                catch (Exception e)
                {
                    Log.Instance.Do(e);
                }
            }
        }



        public void PlayAlarmActive()
        {
            PlayQueued(_alarmActiveRecord);
        }

        public void PlayAlarmOn()
        {
            PlayQueued(_alarmOnRecord);
        }

        public void PlayLabOpen()
        {
            PlayQueued(_labOpenRecord);
        }

        public void PlayGeneratorReady()
        {
            PlayQueued(_generatorReadyRecord);
        }

        public void PlayGunReady()
        {
            PlayQueued(_gunReadyRecord);
        }

        public void PlayGunShotBg()
        {
            PlayQueued(_gunShotBgRecord);
        }

        public void TestDevice()
        {
            _deviceMgr.PlayNow(_boxRecord);
        }
    }
}
