using Technoquest.Api.Common;

namespace Technoquest.Api.Models
{
    public class BookingModel
    {
        public string Date { get; set; }
        public int TimeSlot { get; set; }
        public Quest Quest { get; set; }
    }
}
