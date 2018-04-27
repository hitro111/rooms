namespace MqttTest.Audio
{
    enum AudioEventCode
    {
        Unknown,

        //space
        Air,
        Box,
        CpuOpen,
        CreoDoor,
        Engine,
        GameFail,
        Power,
        Sputnik,
        GameSuccess,
        Voice
    }

    public interface IPlayItem
    {
        string FileName { get; }
        int? EventCode { get; }
        float? Volume { get; set; }
    }

    struct SoundRecord : IPlayItem
    {
        public SoundRecord(string fileName, AudioEventCode? eventCode = null, float? volume = null)
        {
            FileName = fileName;
            EventCode = eventCode;
            Volume = volume;
        }

        public string FileName { get; set; }
        public AudioEventCode? EventCode { get; set; }
        public float? Volume { get; set; }

        int? IPlayItem.EventCode => (int?)EventCode;
    }
}
