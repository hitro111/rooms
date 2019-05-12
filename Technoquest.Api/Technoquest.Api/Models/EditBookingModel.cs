using Technoquest.Api.Common;

namespace Technoquest.Api.Models
{
    public class EditBookingModel
    {
        public string Id { get; set; }
        public BookingStatus Status { get; set; }
    }
}
