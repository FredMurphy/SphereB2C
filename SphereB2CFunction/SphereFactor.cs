using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.Devices.Common.Exceptions;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using SphereB2CFunction.IotHub;
using SphereB2CFunction.Model;
using System;
using System.IO;
using System.Net;
using System.Threading.Tasks;

namespace SphereB2CFunction
{
    /// <summary>
    /// Calls through to our Azure Sphere device (via Azure IoT Hub) for our custom Multi-Factor Authentication
    /// </summary>
    /// <remarks>
    /// Please note this is kept as simple as possible to illustrate a point. It is not production quality code.
    /// </remarks>
    public class SphereFactor
    {
        private IHubService HubService { get; }

        public SphereFactor(IHubService hubService)
        {
            HubService = hubService;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="req">
        ///     The HTTP request
        /// 
        ///     We will expect something similar to this in the request body:
        ///     {
        ///         "name": "Fred",
        ///         "deviceName": "AvnetDevice",
        ///         "secondaryMethod": "nfc",
        ///         "expectedValue": "1234"
        ///     }
        /// </param>
        /// <param name="log">Logger to be used to writing log info</param>
        /// <returns>
        ///     An OkObjectResult with JSON. The following properties are most significant:
        ///         confirmed:  A boolean represening whether out multi-factor authentication was successful
        ///         debug:      Some useful debug information
        /// </returns>
        [FunctionName("Confirm")]
        public async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", "post", Route = null)] HttpRequest req,
            ILogger log)
        {
            log.LogInformation("SphereFactor.Confirm called");

            string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            PostData data = JsonConvert.DeserializeObject<PostData>(requestBody);

            if (data == null)
                return new BadRequestObjectResult("Please pass the required data in the request body");

            // Backdoor for testing - button / button_z will always return "success"
            if (data.secondaryMethod == "button" && data.expectedValue == "button_z")
            {
                return new OkObjectResult(
                      new ResponseContent
                      {
                          version = "1.0.0",
                          status = (int)HttpStatusCode.OK,
                          confirmed = true,
                          debug = "button_z will always always return confirmed"
                      });
            }

            try
            {
                var deviceResponse = await HubService.GetSphereAuthentication(data.deviceName, data.secondaryMethod);

                var confirmed = !deviceResponse.error
                    && data.secondaryMethod.Equals(deviceResponse.method)
                    && data.expectedValue.Equals(deviceResponse.value);

                return new OkObjectResult(
                      new ResponseContent
                      {
                          version = "1.0.0",
                          status = (int)HttpStatusCode.OK,
                          confirmed = confirmed,
                          debug = JsonConvert.SerializeObject(deviceResponse)
                      });
            }
            catch (DeviceNotFoundException ex)
            {
                // Device is probably not powered up. That's OK. 
                log.LogError(ex, $"Device {data.deviceName} not found");

                return new OkObjectResult(
                      new ResponseContent
                      {
                          version = "1.0.0",
                          status = (int)HttpStatusCode.OK,
                          confirmed = false,
                          debug = $"Device {data.deviceName} not found"
                      });
            }
            catch (Exception ex)
            {
                // General error. Fail quietly.
                log.LogError(ex, "Exception during SphereFactor.Confirm");

                return new OkObjectResult(
                      new ResponseContent
                      {
                          version = "1.0.0",
                          status = (int)HttpStatusCode.OK,
                          confirmed = false,
                          debug = "Exception: " + ex.Message
                      });

            }
        }
    }
}
