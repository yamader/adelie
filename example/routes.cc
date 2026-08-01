#include <string>

import adelie;
import example.controllers;

namespace example::middleware {

auto auth(Request& req, Next const& next) -> Response {
  if (req.header("authorization").empty()) return Response::json({{"error", "unauthorized"}}, Status::unauthorized);
  req.set_header("x-user", "root");
  return next();
}

auto verified(Request&, Next const& next) -> Response {
  auto response = next();
  response.set_header("x-verified", "1");
  return response;
}

}

namespace example {

auto define_routes() -> void {
  using adelie::Route;

  Route::alias_middleware("auth", middleware::auth);
  Route::alias_middleware("verified", middleware::verified);

  Route::fallback([](Request const& req) {
    return Response::json({{"error", "not found"}, {"path", req.path()}}, Status::not_found);
  });

  Route::get("/", &HomeController::index).name("home");

  Route::get("/users", &UserController::index).name("users.index");
  Route::get("/users/{id}", &UserController::show).name("users.show").where("id", "[0-9]+");
  Route::post("/users", &UserController::store).name("users.store");

  Route::prefix("admin").name("admin.").middleware({"auth", "verified"}).group([] {
    Route::get("/dashboard", &DashboardController::show).name("dashboard");
    Route::delete_("/users/{id}", &UserController::destroy).name("users.destroy");
  });
}

}
