using Technoquest.Api.Common;

namespace Technoquest.Api.Models
{
    public class BookingFullModel
    {
        public string Date { get; set; }
        public int TimeSlot { get; set; }
        public string Name { get; set; }
        public string Phone { get; set; }
        public string Email { get; set; }
        public string PromoCode { get; set; }
        public string Comment { get; set; }
        public Quest Quest { get; set; }
    }
}
