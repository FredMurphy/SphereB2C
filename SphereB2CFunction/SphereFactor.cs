using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;
using System.IO;
using System.Net;
using System.Threading.Tasks;

namespace SphereB2CFunction
{
    public static class SphereFactor
    {
        [FunctionName("Confirm")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", "post", Route = null)] HttpRequest req,
            ILogger log)
        {
            log.LogInformation("C# HTTP trigger function processed a request.");

            string name = req.Query["name"];

            string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            dynamic data = JsonConvert.DeserializeObject(requestBody);
            name = name ?? data?.name;

            if (name == null)
                return new BadRequestObjectResult("Please pass a name on the query string or in the request body");

            return (ActionResult)new OkObjectResult(
                  new ResponseContent
                  {
                      version = "1.0.0",
                      status = (int)HttpStatusCode.OK,
                      confirmed = true
                  }) ;
        }
    }
}
