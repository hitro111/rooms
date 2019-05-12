using System;
using System.Threading.Tasks;
using MongoRepositories.Mongo;
using Technoquest.Api.Infrastructure.Mongo;

namespace Technoquest.Api.Repositories
{
    public class CallBackEntity : MongoEntity
    {
        public string Phone { get; set; }
        public DateTime Date { get; set; }
        public string Reason { get; set; }


        public static CallBackEntity Create(string phone, string reason)
        {
            return new CallBackEntity
            {
                BsonId = Guid.NewGuid().ToString(),
                Date = DateTime.UtcNow,
                Reason = reason
            };
        }
    }

    public class CallBackRepository
    {
        private readonly IMongoStorage<CallBackEntity> _storage;

        public CallBackRepository(IMongoStorage<CallBackEntity> storage)
        {
            _storage = storage;
        }

        public async Task<CallBackEntity> Create(string phone, string reason)
        {
            var rec = CallBackEntity.Create(phone, reason);
            await _storage.InsertAsync(rec);
            return rec;
        }

        public async Task Clear()
        {
            await _storage.DeleteAsync(x => true);
        }
    }
}
