using MongoDB.Driver;

namespace Technoquest.Api.Infrastructure.Mongo
{
	public static class MongoExceptionExtensions
	{
		public static bool IsDuplicateError(this MongoServerException ex)
		{
			return ex.Message.Contains("duplicate key error");
		}
	}
}
