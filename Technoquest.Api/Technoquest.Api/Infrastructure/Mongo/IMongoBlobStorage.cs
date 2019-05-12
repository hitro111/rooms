using System;
using System.IO;
using System.Threading.Tasks;

namespace Technoquest.Api.Infrastructure.Mongo
{
    public interface IMongoBlobStorage
    {
        Task<string> SaveBlobAsync(string container, string key, Stream stream);

        Task<Stream> GetAsync(string container, string key);
	    Task<bool> HasBlobAsync(string containerName, string blobKey);
	    Task<DateTime?> GetBlobsLastModifiedAsync(string containerName);
		Task<string> SaveBlobAsync(string containerName, string fileName, byte[] v);
	    Task<string> GetAsTextAsync(string containerName, string fileName);
    }
}
