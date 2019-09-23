using System.Threading.Tasks;

namespace SphereB2CFunction.IotHub
{
    public interface IHubService
    {
        Task<string> GetSphereAuthentication(string deviceName, string secondaryMethod);
    }
}
