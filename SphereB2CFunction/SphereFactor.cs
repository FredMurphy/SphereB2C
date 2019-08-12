using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using SphereB2CFunction.IotHub;
using SphereB2CFunction.Model;
using System.IO;
using System.Net;
using System.Threading.Tasks;

namespace SphereB2CFunction
{
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
        /// <param name="req"></param>
        /// <param name="log"></param>
        /// {
        ///     "name": "Fred",
        ///     "deviceName": "AvnetDevice",
        ///     "secondaryMethod": "nfc",
        ///     "expectedValue": "1234"
        /// }
        /// <returns></returns>
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

            var response = await HubService.GetSphereAuthentication(data.deviceName, data.secondaryMethod, "helloOOOoooOOOoo");

            var deviceResponse = JsonConvert.DeserializeObject<DeviceResponse>(response);

            var confirmed = !deviceResponse.error
                && data.secondaryMethod.Equals(deviceResponse.method)
                && data.expectedValue.Equals(deviceResponse.value);

            return (ActionResult)new OkObjectResult(
                  new ResponseContent
                  {
                      version = "1.0.0",
                      status = (int)HttpStatusCode.OK,
                      confirmed = confirmed,
                      debug = response
                  }) ;
        }
    }
}
