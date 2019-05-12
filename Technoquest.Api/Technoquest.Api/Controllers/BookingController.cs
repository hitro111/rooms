using System;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Options;
using Technoquest.Api.Infrastructure;
using Technoquest.Api.Models;
using Technoquest.Api.Repositories;
using Technoquest.Api.Services;
using Telegram.Bot;
using Telegram.Bot.Types;
using Telegram.Bot.Types.Enums;

namespace Technoquest.Api.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class BookingController : ControllerBase
    {
        private readonly BookingRepository _bookingRepository;
        private readonly TelegramBotClient _botClient;
        private readonly IOptions<ApiSettings> _apiSettings;

        public BookingController(BookingRepository bookingRepository, TelegramBotClient botClient,
            IOptions<ApiSettings> apiSettings)
        {
            _bookingRepository = bookingRepository;
            _botClient = botClient;
            _apiSettings = apiSettings;
        }

        [HttpGet]
        public async Task<IActionResult> Get([FromQuery] string dateFrom, [FromQuery] string dateTo)
        {
            var bookings = await _bookingRepository.GetBookings(dateFrom, dateTo);

            var model = bookings.Select(x => new BookingModel
            {
                Quest = x.Quest,
                Date = x.Date.ToString("dd'/'MM'/'yyyy"),
                TimeSlot = x.TimeSlot
            }).ToArray();

            return Ok(model);
        }

        [HttpPost]
        public async Task<IActionResult> Post([FromBody] BookingFullModel booking)
        {
            var rec = await _bookingRepository.Create(booking.Quest, booking.Date, booking.TimeSlot, booking.Name, booking.Phone,
                booking.Email,
                booking.PromoCode, booking.Comment);

            var chatId = _apiSettings.Value.TelegramChatId;

#pragma warning disable 4014
            _botClient.SendTextMessageAsync(new ChatId(chatId),
#pragma warning restore 4014
                $"*New game! [{rec.BsonId}]*\n" +
                $"_Quest_: {booking.Quest}, _Date_: {booking.Date}, _Time_: {TimeslotHelper.ToTime(booking.TimeSlot, booking.Quest)}\n" +
                $"_Name_: {booking.Name}, _Email_: {booking.Email}, _Phone_: *{booking.Phone}* \n" +
                $"_Promo Code_: {booking.PromoCode}\n" +
                $"_Comment_: {booking.Comment}",
                ParseMode.Markdown);


            return Ok();
        }

        //ToDo: remove
        [HttpDelete]
        public async Task<IActionResult> Delete()
        {
            await _bookingRepository.Clear();
            return Ok();
        }

        [HttpPut]
        public async Task<IActionResult> ChangeStatus(EditBookingModel model)
        {
            await _bookingRepository.ChangeStatus(model.Id, model.Status);

            return Ok();
        }
    }
}