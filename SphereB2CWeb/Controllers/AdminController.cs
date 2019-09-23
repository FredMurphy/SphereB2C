using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace SphereB2CWeb.Controllers
{
    public class AdminController : Controller
    {
        [Authorize(Policy ="IsMultiFactorAuthenticated")]
        public IActionResult Index()
        {
            return View();
        }
    }
}