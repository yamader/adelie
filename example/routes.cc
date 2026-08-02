#include <memory>
#include <string>
#include <string_view>

import adelie;
import example.controllers;
import example.auth;

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
  using adelie::Route;

  example::auth::setup();

  Route::alias_middleware("auth", adelie::auth::authenticator().middleware());
  Route::alias_middleware("verified", middleware::verified);

  Route::fallback([](Request const& req) {
    return Response::json({{"error", "not found"}, {"path", req.path()}}, Status::not_found);
  });

  Route::get("/", &HomeController::index).name("home");

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
