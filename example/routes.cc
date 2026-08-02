import std;
import adelie;
import example.auth;
import example.controllers;

namespace example::middleware {

using Next = adelie::support::Callable<adelie::http::Response()>;

auto verified(Request&, Next const& next) -> Response {
  auto response = next();
  response.set_header("x-verified", "1");
  return response;
}

}

namespace example {

auto define_routes() -> void {
  example::auth::setup();

  Route::alias_middleware("auth", adelie::auth::authenticator().middleware());
  Route::alias_middleware("verified", middleware::verified);

  Route::fallback([](Request const& req) {
    return Response::json({{"error", "not found"}, {"path", req.path()}}, Status::not_found);
  });

  Route::get("/", &HomeController::index).name("home");

  Route::get("/hello", [](Request const& req) {
    return view("home", {{"app", Config::get("app.name")},
                         {"name", std::string{req.query("name", "world")}},
                         {"admin", req.query("admin") == "1"},
                         {"note", std::string{"rendered by <code>adelie.view</code>"}}});
  }).name("hello");

  Route::get("/assets/{path*}", [](Request const& req) {
    auto const asset = adelie::Resource::find(adelie::resource::root() / "assets", req.route("path"));
    if (!asset) return Response::text("not found", Status::not_found);
    return asset->response();
  }).name("assets");

  Route::get("/users", &UserController::index).name("users.index");
  Route::get("/users/{id}", &UserController::show).name("users.show").where("id", "[0-9]+");
  Route::post("/users", &UserController::store).name("users.store");

  Route::post("/login", &adelie::auth::login);
  Route::post("/logout", &adelie::auth::logout);

  Route::prefix("admin").name("admin.").middleware({"auth", "verified"}).group([] {
    Route::get("/dashboard", &DashboardController::show).name("dashboard");
    Route::delete_("/users/{id}", &UserController::destroy).name("users.destroy");
  });
}

}
