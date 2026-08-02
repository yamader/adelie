import std;
import adelie;
import example.config;
import example.controllers;
import example.models;

namespace example {

auto define_routes() -> void;

}

using adelie::db::connect;
using adelie::db::execute;
using example::User;

namespace {

auto seed_database() -> void {
  connect(adelie::db::DatabaseConfig{.connection = ":memory:", .options = {{"foreign_keys", "1"}}});
  execute(
      "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, "
      "created_at TEXT NOT NULL, updated_at TEXT NOT NULL)");
  User::create({{"name", "Alice"}});
  User::create({{"name", "Bob"}});
  User::create({{"name", "Carol"}});
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
  auto const users = User::all();
  std::printf("\nusers in database: %zu\n", users.size());
  for (auto const& user : users) {
    std::printf("  - id=%lld name=%s\n", static_cast<long long>(user.key()),
                std::string(user.get("name").as_string()).c_str());
  }
}

auto print_config() -> void {
  std::printf("\nconfig: app.name=%s host=%s port=%lld debug=%s\n", Config::get("app.name").c_str(),
              Config::get("host").c_str(), static_cast<long long>(Config::get_int("port")),
              Config::get_bool("debug") ? "true" : "false");
  std::printf("env: HOME=%s\n", adelie::support::env("HOME", "?").c_str());
}

auto serve() -> int {
  adelie::Server server{Route::router()};
  server.workers(4).listen("127.0.0.1", 8080);
  std::printf("adelie listening on http://%s:%u\n", std::string(server.address()).c_str(), server.port());
  server.run();
  return 0;
}

}

auto main(int argc, char** argv) -> int {
  example::setup_config();
  adelie::di::install<example::HomeController, example::UserController, example::DashboardController>(
      adelie::di::make(adelie::di::bind<example::UserSource>().to<example::UserRepository>(),
                       adelie::di::bind<example::Logger>().in(adelie::di::singleton)));

  example::define_routes();
  seed_database();

  if (argc > 1 && std::string_view(argv[1]) == "serve") return serve();

  print_table();
  print_urls();
  print_db();
  print_config();
}
