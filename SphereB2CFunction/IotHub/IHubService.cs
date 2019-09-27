using SphereB2CFunction.Model;
using System.Threading.Tasks;

namespace SphereB2CFunction.IotHub
{
    public interface IHubService
    {
        Task<DeviceResponse> GetSphereAuthentication(string deviceName, string secondaryMethod);
    }
}
