using Technoquest.Api.Common;

namespace Technoquest.Api.Models
{

    public class TimesSlotModel
    {
        public Quest Quest { get; set; }
        public TimeSlotPrice[] Pricing { get; set; }
    }

    public class TimeSlotPrice
    {
        public int TimeSlot { get; set; }
        public PriceType PriceType { get; set; }
    }
}
