using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using System;
using System.Threading.Tasks;

namespace SphereB2CFunction.IotHub
{

    public class HubService : IHubService
    {
        //TODO: Get from secrets
        private static readonly string ConnectionString = "HostName=0xfred-hub.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=4eoTO7u5hZpFaXJh6KtaRrg+6195w/ZJi27OSJbbNY8=";

        public async Task<string> GetSphereAuthentication(string deviceName, string secondaryMethod, string message)
        {
            var serviceClient = ServiceClient.CreateFromConnectionString(ConnectionString);

            var methodInvocation = new CloudToDeviceMethod("Authenticate") { ResponseTimeout = TimeSpan.FromSeconds(30) };

            var payload = new { deviceName, secondaryMethod };
            methodInvocation.SetPayloadJson(JsonConvert.SerializeObject(payload));


            var response = await serviceClient.InvokeDeviceMethodAsync(deviceName, methodInvocation);

            if (response.Status == 200)
                return response.GetPayloadAsJson();

            return "\"FAIL\"";

        }

    }
}
