using System;
using System.Collections.Generic;
using NAudio.Wave;

namespace MqttTest.Audio
{
    public class BackgroundAudio
    {
        private readonly string _fileName;

        public BackgroundAudio(string fileName)
        {
            _fileName = fileName;
        }

        private WaveOut[] _waveOuts = new WaveOut[]
        {
            //new WaveOut {DeviceNumber = 0},
            new WaveOut {DeviceNumber = 1},
            new WaveOut {DeviceNumber = 2},

            new WaveOut {DeviceNumber = 3}
        };

        public void PlaySoundInDevice(int deviceNumber)
        {
            try
            {
                var w = _waveOuts[deviceNumber];
                w.Stop();

                if (outputDevices.ContainsKey(deviceNumber))
                {
                    outputDevices[deviceNumber].WaveOut.Dispose();
                    outputDevices[deviceNumber].WaveStream.Dispose();
                }

                WaveStream waveStream = IsMp3(_fileName)
                    ? (WaveStream)new Mp3FileReader(_fileName)
                    : new WaveFileReader(_fileName);

                LoopStream loop = new LoopStream(waveStream);

                // hold onto the WaveOut and  WaveStream so we can dispose them later
                outputDevices[deviceNumber] = new PlaybackSession { WaveOut = w, WaveStream = loop };

                w.Init(loop);
                w.Play();
            }
            catch (Exception)
            {
                //
            }
        }

        private bool IsMp3(string fileName)
        {
            return fileName.ToLower().EndsWith(".mp3");
        }

        private Dictionary<int, PlaybackSession> outputDevices = new Dictionary<int, PlaybackSession>();

        public void StopAll()
        {
            foreach (var playbackSession in outputDevices.Values)
            {
                playbackSession.WaveOut.Stop();
            }
        }

        public void DisposeAll()
        {
            foreach (var playbackSession in outputDevices.Values)
            {
                playbackSession.WaveOut.Stop();
                playbackSession.WaveOut.Dispose();
                playbackSession.WaveStream.Dispose();
            }
        }

        class PlaybackSession
        {
            public IWavePlayer WaveOut { get; set; }
            public WaveStream WaveStream { get; set; }
        }

        public void PlayBackground()
        {
            int waveOutDevices = WaveOut.DeviceCount;
            for (int n = 0; n < waveOutDevices; n++)
            {
                PlaySoundInDevice(n);
            }
        }
    }
}
