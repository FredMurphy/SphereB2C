using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using SphereB2CFunction.Model;
using System;
using System.Threading.Tasks;

namespace SphereB2CFunction.IotHub
{

    /// <summary>
    /// Wraps direct method invocation on our device
    /// </summary>
    public class HubService : IHubService
    {
        // Get Hub Connection string from Function appSettings
        private static readonly string HubConnectionString = System.Environment.GetEnvironmentVariable("HubConnectionString", EnvironmentVariableTarget.Process);

        /// <summary>
        /// Call Authenticate
        /// </summary>
        /// <param name="deviceName">Device name as registered with Azure IoT Hub</param>
        /// <param name="secondaryMethod">Authentication method - "button" or "nfc"</param>
        /// <returns>a DeviceResponse object</returns>
        public async Task<DeviceResponse> GetSphereAuthentication(string deviceName, string secondaryMethod)
        {
            // Setup "Authenticate" direct method
            var serviceClient = ServiceClient.CreateFromConnectionString(HubConnectionString);
            var methodInvocation = new CloudToDeviceMethod("Authenticate") { ResponseTimeout = TimeSpan.FromSeconds(30) };

            // Pass in our secondaryMethod parameter as JSON
            var payload = new { secondaryMethod };
            methodInvocation.SetPayloadJson(JsonConvert.SerializeObject(payload));

            // Invoke method
            var response = await serviceClient.InvokeDeviceMethodAsync(deviceName, methodInvocation);

            // Return the JSON we get from the Azure Sphere device
            if (response.Status == 200)
                return JsonConvert.DeserializeObject<DeviceResponse>(response.GetPayloadAsJson());

            return new DeviceResponse {
                error = true,
                method = "fail",
                value = $"response status {response.Status}"
            };

        }

    }
}
