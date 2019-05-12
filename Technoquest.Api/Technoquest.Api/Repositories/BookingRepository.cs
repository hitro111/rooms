using System;
using System.Globalization;
using System.Linq;
using System.Threading.Tasks;
using MongoRepositories.Mongo;
using Technoquest.Api.Common;
using Technoquest.Api.Infrastructure.Mongo;

namespace Technoquest.Api.Repositories
{
    public class BookingEntity : MongoEntity
    {
        public Quest Quest { get; set; }
        public DateTime Date { get; set; }
        public int TimeSlot { get; set; }
        public string Name { get; set; }
        public string Phone { get; set; }
        public string Email { get; set; }
        public string PromoCode { get; set; }
        public string Comment { get; set; }
        public BookingStatus Status { get; set; }

        public static BookingEntity Create(Quest quest, DateTime date, int timeSlot, string name, string phone, string email, string promoCode, string comment)
        {
            return new BookingEntity
            {
                BsonId = Guid.NewGuid().ToString(),
                Quest = quest,
                Date = date,
                TimeSlot = timeSlot,
                Name = name,
                Phone = phone,
                Email = email,
                PromoCode = promoCode,
                Comment = comment,
            };
        }
    }

    public class BookingRepository
    {
        private readonly IMongoStorage<BookingEntity> _storage;

        public BookingRepository(IMongoStorage<BookingEntity> storage)
        {
            _storage = storage;
        }

        public async Task<BookingEntity> Create(Quest quest, string date, int timeSlot, string name, string phone, string email, string promoCode, string comment)
        {
            var dt = DateTime.ParseExact(date, "dd/MM/yyyy", CultureInfo.InvariantCulture, DateTimeStyles.AssumeUniversal);
            var rec = BookingEntity.Create(quest, dt, timeSlot, name, phone, email, promoCode, comment);
            await _storage.InsertAsync(rec);
            return rec;
        }

        public async Task<BookingEntity[]> GetBookings(string dateFrom, string dateTo)
        {
            DateTime from = DateTime.ParseExact(dateFrom, "dd/MM/yyyy", CultureInfo.InvariantCulture);
            DateTime to = DateTime.ParseExact(dateTo, "dd/MM/yyyy", CultureInfo.InvariantCulture).AddDays(1);

            var data = await _storage.GetDataAsync(x => x.Date >= from && x.Date <= to && x.Status == BookingStatus.Confirmed);

            return data.ToArray();
        }

        public async Task<BookingEntity> ChangeStatus(string id, BookingStatus status)
        {
            var rec = await _storage.GetDataAsync(id);

            return await _storage.ReplaceAsync(id, x =>
            {
                x.Status = status;
                return x;
            });
        }

        public async Task Clear()
        {
            await _storage.DeleteAsync(x => true);
        }
    }
}
