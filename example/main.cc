#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

import adelie;
import example.controllers;

namespace example {
auto define_routes() -> void;
}

using adelie::Config;
using adelie::DB;
using adelie::Route;
using adelie::Value;

namespace {

auto seed_database() -> void {
  DB::connect(Config{.connection = ":memory:", .options = {{"foreign_keys", "1"}}});
  DB::execute("CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL)");
  DB::transaction([] {
    DB::execute("INSERT INTO users (name) VALUES (?)", {Value{"Alice"}});
    DB::execute("INSERT INTO users (name) VALUES (?)", {Value{"Bob"}});
    DB::execute("INSERT INTO users (name) VALUES (?)", {Value{"Carol"}});
  });
}

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
  std::printf("users.index          -> %s\n", Route::url("users.index").c_str());
  std::printf("admin.users.destroy  -> %s\n", Route::url("admin.users.destroy", {{"id", "7"}}).c_str());

  auto const* show = Route::named("users.show");
  std::printf("\nusers.show where(id) = %s\n", std::string(show->constraint("id")).c_str());
}

auto print_db() -> void {
  std::printf("\nusers in database: %zu\n", DB::execute("SELECT id, name FROM users ORDER BY id").size());
  for (auto const& row : DB::execute("SELECT id, name FROM users ORDER BY id")) {
    std::printf("  - id=%lld name=%s\n", static_cast<long long>(row[0].as_int()),
                std::string(row["name"].as_string()).c_str());
  }
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
  adelie::di::install<example::HomeController, example::UserController, example::DashboardController>(
      adelie::di::make(adelie::di::bind<example::UserSource>().to<example::UserRepository>(),
                       adelie::di::bind<example::Logger>().in(adelie::di::singleton)));

  example::define_routes();
  seed_database();

  if (argc > 1 && std::string_view(argv[1]) == "serve") return serve();

  print_table();
  print_urls();
  print_db();
}
