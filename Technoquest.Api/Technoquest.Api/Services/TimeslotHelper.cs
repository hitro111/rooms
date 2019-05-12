using Technoquest.Api.Common;

namespace Technoquest.Api.Services
{
    public static class TimeslotHelper
    {
        public static string ToTime(int timeSlot, Quest quest)
        {
            if (quest == Quest.Zarmina)
            {
                switch (timeSlot)
                {
                    case 1:
                        return "09:30";
                    case 2:
                        return "11:00";
                    case 3:
                        return "12:30";
                    case 4:
                        return "14:00";
                    case 5:
                        return "15:30";
                    case 6:
                        return "17:00";
                    case 7:
                        return "18:30";
                    case 8:
                        return "20:00";
                    case 9:
                        return "21:30";
                    case 10:
                        return "23:00";

                }
            }

            if (quest == Quest.Ter2070)
            {
                switch (timeSlot)
                {
                    case 1:
                        return "10:00";
                    case 2:
                        return "11:30";
                    case 3:
                        return "13:00";
                    case 4:
                        return "14:30";
                    case 5:
                        return "16:00";
                    case 6:
                        return "17:30";
                    case 7:
                        return "19:00";
                    case 8:
                        return "20:30";
                    case 9:
                        return "22:00";
                    case 10:
                        return "23:30";
                }
            }

            return "unknown";
        }
    }
}
