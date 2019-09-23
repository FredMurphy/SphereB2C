using Microsoft.AspNetCore.Mvc;

namespace SphereB2CWeb.Controllers
{
    public class UserController : Controller
    {
        public IActionResult Index()
        {
            return View();
        }
    }
}