using System;
using NAudio.Wave;

namespace MqttTest.Audio
{
    public class BackgroundAudio
    {
        private string _fileName;
        private readonly WaveOut _waveOut;

        public BackgroundAudio(string fileName, int channel)
        {
            _fileName = fileName;
            _waveOut = new WaveOut()
            {
                DeviceNumber = channel
            };
        }

        public void ChangeTrack(string fileName)
        {
            _fileName = fileName;
            PlaySound();
        }

        public void PlaySound()
        {
            try
            {
                _waveOut?.Stop();

                outputDevice?.WaveOut?.Dispose();
                outputDevice?.WaveStream?.Dispose();
                

                WaveStream waveStream = IsMp3(_fileName)
                     ? (WaveStream)new Mp3FileReader(_fileName)
                    : new WaveFileReader(_fileName);

                LoopStream loop = new LoopStream(waveStream);

                // hold onto the WaveOut and  WaveStream so we can dispose them later
                outputDevice = new PlaybackSession { WaveOut = _waveOut, WaveStream = loop };

                _waveOut.Init(loop);
                _waveOut.Play();
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

        private PlaybackSession outputDevice;

        public void Stop()
        {
            _waveOut?.Stop();
        }

        public void DisposeAll()
        {
            outputDevice?.WaveOut?.Stop();
            outputDevice?.WaveOut?.Dispose();
            outputDevice?.WaveStream?.Dispose();
        }

        class PlaybackSession
        {
            public IWavePlayer WaveOut { get; set; }
            public WaveStream WaveStream { get; set; }
        }
    }
}
