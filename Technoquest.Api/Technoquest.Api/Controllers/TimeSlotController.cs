using System;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Options;
using Technoquest.Api.Common;
using Technoquest.Api.Infrastructure;
using Technoquest.Api.Models;
using System.Globalization;

namespace Technoquest.Api.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class TimeSlotController : ControllerBase
    {
        public TimeSlotController()
        {
        }

        [HttpGet]
        public async Task<IActionResult> Get(string date)
        {
            var dt = DateTime.ParseExact(date, "dd/MM/yyyy", CultureInfo.InvariantCulture,
                DateTimeStyles.AssumeUniversal);

            TimesSlotModel[] model = null;

            if (dt.DayOfWeek == DayOfWeek.Saturday | dt.DayOfWeek == DayOfWeek.Sunday)
            {
                model = new TimesSlotModel[]
            {
                new TimesSlotModel
                {

                    Quest = Quest.Zarmina,
                    Pricing = new TimeSlotPrice[]
                    {
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 0
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 1
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 2
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 3
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 4
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 5
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 6
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 7
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 8
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 9
                        }
                    }
                },

                new TimesSlotModel
                {

                    Quest = Quest.Ter2070,
                    Pricing = new TimeSlotPrice[]
                    {
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 0
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 1
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 2
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 3
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 4
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 5
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 6
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 7
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 8
                        },
                        new TimeSlotPrice
                        {
                            PriceType = PriceType.High,
                            TimeSlot = 9
                        }
                    }
                }
            };
            }
            else
            {

                model = new TimesSlotModel[]
                {
                    new TimesSlotModel
                    {

                        Quest = Quest.Zarmina,
                        Pricing = new TimeSlotPrice[]
                        {
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 0
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 1
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 2
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 3
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 4
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 5
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 6
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 7
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 8
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 9
                            }
                        }
                    },

                    new TimesSlotModel
                    {

                        Quest = Quest.Ter2070,
                        Pricing = new TimeSlotPrice[]
                        {
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 0
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 1
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 2
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 3
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 4
                            },
                            new TimeSlotPrice
                            {
                                PriceType = 0,
                                TimeSlot = 5
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 6
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 7
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 8
                            },
                            new TimeSlotPrice
                            {
                                PriceType = PriceType.High,
                                TimeSlot = 9
                            }
                        }
                    }
                };
            }

            return Ok(model);
        }
    }
}