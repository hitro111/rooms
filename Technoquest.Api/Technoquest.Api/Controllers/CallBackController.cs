using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Options;
using Technoquest.Api.Infrastructure;
using Technoquest.Api.Models;
using Technoquest.Api.Repositories;
using Telegram.Bot;
using Telegram.Bot.Types;
using Telegram.Bot.Types.Enums;

namespace Technoquest.Api.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class CallBackController : ControllerBase
    {
        private readonly CallBackRepository _callBackRepository;
        private readonly TelegramBotClient _botClient;
        private readonly IOptions<ApiSettings> _apiSettings;

        public CallBackController(CallBackRepository callBackRepository, TelegramBotClient botClient,
            IOptions<ApiSettings> apiSettings)
        {
            _callBackRepository = callBackRepository;
            _botClient = botClient;
            _apiSettings = apiSettings;
        }

        [HttpPost]
        public async Task<IActionResult> Post([FromBody] CallBackModel callBack)
        {
            await _callBackRepository.Create(callBack.Phone, callBack.Reason);

            var chatId = _apiSettings.Value.TelegramChatId;

#pragma warning disable 4014
            _botClient.SendTextMessageAsync(new ChatId(chatId),
#pragma warning restore 4014
                $"*Callback!*\n" +
                $"_Phone_: *{callBack.Phone}*\n" +
                $"_Reason_: {callBack.Reason}",
                ParseMode.Markdown);


            return Ok();
        }
    }
}