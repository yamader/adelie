#include <cstdio>
#include <string>

import adelie;
import example.routes;

using adelie::Route;

namespace {

auto print_table() -> void {
  std::printf("%-16s %-24s %-24s %-22s %s\n", "METHODS", "URI", "BOOST PATTERN", "NAME", "MIDDLEWARE");
  for (auto const& route : Route::routes()) {
    std::string middleware;
    for (auto const& m : route.middleware()) {
      if (!middleware.empty()) middleware += ',';
      middleware += m;
    }
    std::printf("%-16s %-24s %-24s %-22s %s\n", adelie::http::to_string(route.methods()).c_str(),
                std::string(route.uri()).c_str(), route.pattern().to_boost_pattern().c_str(),
                std::string(route.name()).c_str(), middleware.c_str());
  }
}

}

auto main() -> int {
  example::define_routes();
  print_table();

  std::printf("\nusers.show           -> %s\n", Route::url("users.show", {{"id", "42"}}).c_str());
  std::printf("posts.show (no slug) -> %s\n", Route::url("posts.show", {{"year", "2026"}}).c_str());
  std::printf("posts.show (slug)    -> %s\n",
              Route::url("posts.show", {{"year", "2026"}, {"slug", "hello world"}}).c_str());
  std::printf("assets               -> %s\n", Route::url("assets", {{"path", "css/app.css"}}).c_str());
  std::printf("admin.users.destroy  -> %s\n", Route::url("admin.users.destroy", {{"id", "7"}}).c_str());

  auto const* show = Route::named("users.show");
  std::printf("\nusers.show where(id) = %s\n", std::string(show->constraint("id")).c_str());
}
