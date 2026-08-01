#include <cstdio>
#include <string>
#include <string_view>

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

auto print_urls() -> void {
  std::printf("\nusers.show           -> %s\n", Route::url("users.show", {{"id", "42"}}).c_str());
  std::printf("posts.show (no slug) -> %s\n", Route::url("posts.show", {{"year", "2026"}}).c_str());
  std::printf("posts.show (slug)    -> %s\n",
              Route::url("posts.show", {{"year", "2026"}, {"slug", "hello world"}}).c_str());
  std::printf("assets               -> %s\n", Route::url("assets", {{"path", "css/app.css"}}).c_str());
  std::printf("admin.users.destroy  -> %s\n", Route::url("admin.users.destroy", {{"id", "7"}}).c_str());

  auto const* show = Route::named("users.show");
  std::printf("\nusers.show where(id) = %s\n", std::string(show->constraint("id")).c_str());
}

auto serve() -> int {
  adelie::Server server(Route::router());
  server.workers(4).listen("127.0.0.1", 8080);
  std::printf("adelie listening on http://%s:%u\n", std::string(server.address()).c_str(), server.port());
  server.run();
  return 0;
}

}

auto main(int argc, char** argv) -> int {
  example::define_routes();

  if (argc > 1 && std::string_view(argv[1]) == "serve") return serve();

  print_table();
  print_urls();
}
