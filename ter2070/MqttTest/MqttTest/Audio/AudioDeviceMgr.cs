using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using NAudio.Wave;

namespace MqttTest.Audio
{
    public class AudioDeviceMgr
    {
        private readonly LinkedList<IPlayItem> _playList = new LinkedList<IPlayItem>();
        private AudioDevice _device;
        private readonly object _lockObj = new object();

        public AudioDeviceMgr(AudioDevice device)
        {
            _device = device;
        }

        public void ChangeChannel(int deviceNum)
        {
            _device.DeviceNumber = deviceNum;
        }

        public void PlayNow(IPlayItem playItem)
        {
            lock (_lockObj)
            {
                try
                {
                    _playList.Clear();
                    _device.Stop();
                    _playList.AddLast(playItem);
                }
                catch (Exception e)
                {
                    Log.Instance.Do(e);
                }
            }
        }

        public void PlayQueued(IPlayItem playItem)
        {
            lock (_lockObj)
            {
                try
                {

                    RemoveSameEvents(playItem.EventCode);
                    _playList.AddLast(playItem);
                }
                catch (Exception e)
                {
                    Log.Instance.Do(e);
                }
            }
        }

        private void RemoveSameEvents(int? eventCode)
        {
            if (eventCode != null)
            {
                if (_device.IsPlaying && _device.EventCode == eventCode)
                {
                    _device.Stop();
                }

                var itemsToRemove = new List<IPlayItem>();
                foreach (var item in _playList)
                {
                    if (item.EventCode == eventCode)
                        itemsToRemove.Add(item);
                }

                foreach (var item in itemsToRemove)
                {
                    _playList.Remove(item);
                }
            }
        }

        public void Run()
        {
            while (true)
            {
                lock (_lockObj)
                {
                    try
                    {

                        if (!_device.IsPlaying && _playList.Any())
                        {
                            _device.Play(_playList.First.Value);
                            _playList.RemoveFirst();
                        }
                    }
                    catch (Exception e)
                    {
                        Log.Instance.Do(e);
                    }
                }

                Thread.Sleep(20);
            }
        }
    }

    public class AudioDevice : IDisposable
    {
        public IWavePlayer WaveOut { get; set; }
        public WaveStream WaveStream { get; set; }
        public bool IsPlaying;
        public int? EventCode;

        public int DeviceNumber
        {
            set
            {
                WaveOut?.Stop();
                //WaveOut?.Dispose();
                //WaveOut?.Dispose();
                WaveOut = new WaveOut { DeviceNumber = value };
            }
        }

        public AudioDevice(int deviceNum)
        {
            DeviceNumber = deviceNum;
        }

        public void Play(IPlayItem item)
        {
            try
            {
                Stop();
                IsPlaying = true;
                EventCode = item.EventCode;

                WaveStream = IsMp3(item.FileName)
                    ? (WaveStream)new Mp3FileReader(item.FileName)
                    : new WaveFileReader(item.FileName);

                WaveOut.Init(WaveStream);
#pragma warning disable 618
                WaveOut.Volume = item.Volume ?? 1f;
#pragma warning restore 618
                WaveOut.PlaybackStopped += WaveOut_PlaybackStopped;
                WaveOut.Play();
            }
            catch (Exception ex)
            {
                IsPlaying = false;
                EventCode = null;
            }
        }

        public void Stop()
        {
            IsPlaying = false;
            EventCode = null;
            WaveOut.Stop();
            //WaveOut.Dispose();
            WaveStream?.Dispose();
        }

        private void WaveOut_PlaybackStopped(object sender, StoppedEventArgs e)
        {
            IsPlaying = false;
        }

        private bool IsMp3(string fileName)
        {
            return fileName.ToLower().EndsWith(".mp3");
        }

        public void Dispose()
        {
            WaveOut?.Stop();
            //WaveOut?.Dispose();
            WaveStream?.Dispose();
        }
    }
}
