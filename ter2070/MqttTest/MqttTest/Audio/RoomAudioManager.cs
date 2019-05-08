using System;

namespace MqttTest.Audio
{
    public class RoomAudioManager
    {
        private readonly SoundRecord _alarmActiveRecord = new SoundRecord("Resources/Sounds/alarm_activated.mp3", AudioEventCode.Alarm);
        private readonly SoundRecord _alarmOnRecord = new SoundRecord("Resources/Sounds/alarm_on.wav", AudioEventCode.Alarm);
        private readonly SoundRecord _labOpenRecord = new SoundRecord("Resources/Sounds/lab_open.mp3", AudioEventCode.LabDoor);
        private readonly SoundRecord _generatorReadyRecord = new SoundRecord("Resources/Sounds/generator_om.mp3", AudioEventCode.Generator);
        private readonly SoundRecord _gunReadyRecord = new SoundRecord("Resources/Sounds/gun_ready.wav", AudioEventCode.Gun);
        private readonly SoundRecord _gunShotBgRecord = new SoundRecord("Resources/Sounds/gun_shotbg.wav", AudioEventCode.Gun);
        private readonly SoundRecord _smokeOnRecord = new SoundRecord("Resources/Sounds/alarm_activated.mp3", AudioEventCode.Alarm);


        private readonly SoundRecord _boxRecord = new SoundRecord("Resources/Sounds/box_click.wav", AudioEventCode.Box);

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

        public void PlaySmokeOn()
        {
            PlayQueued(_smokeOnRecord);
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
