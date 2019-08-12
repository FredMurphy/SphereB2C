using Microsoft.Azure.Functions.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection;
using SphereB2CFunction.IotHub;

[assembly: FunctionsStartup(typeof(SphereB2CFunction.Startup))]
namespace SphereB2CFunction
{
    public class Startup : FunctionsStartup
    {
        public override void Configure(IFunctionsHostBuilder builder)
        {
            builder.Services.AddSingleton<IHubService, HubService>();
        }
    }
}
