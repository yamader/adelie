module;

#include <cctype>
#include <charconv>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

module adelie.facades.config;

namespace adelie::facades {
namespace detail {

auto values() -> std::map<std::string, std::string>& {
  static std::map<std::string, std::string> instance;
  return instance;
}

auto defaults() -> std::map<std::string, std::string>& {
  static std::map<std::string, std::string> instance;
  return instance;
}

auto lock() -> std::mutex& {
  static std::mutex instance;
  return instance;
}

auto parse_bool(std::string_view value) -> std::optional<bool> {
  std::string lowered;
  lowered.reserve(value.size());
  for (char const c : value) lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  if (lowered == "1" || lowered == "true" || lowered == "yes" || lowered == "on") return true;
  if (lowered == "0" || lowered == "false" || lowered == "no" || lowered == "off") return false;
  return std::nullopt;
}

}

auto Config::defaults(std::initializer_list<std::pair<std::string_view, std::string_view>> entries) -> void {
  std::lock_guard<std::mutex> guard(detail::lock());
  for (auto const& [key, value] : entries) detail::defaults()[std::string(key)] = std::string(value);
}

auto Config::set(std::string_view key, std::string value) -> void {
  std::lock_guard<std::mutex> guard(detail::lock());
  detail::values()[std::string(key)] = std::move(value);
}

auto Config::unset(std::string_view key) -> void {
  std::lock_guard<std::mutex> guard(detail::lock());
  detail::values().erase(std::string(key));
}

auto Config::clear() -> void {
  std::lock_guard<std::mutex> guard(detail::lock());
  detail::values().clear();
  detail::defaults().clear();
}

auto Config::has(std::string_view key) -> bool {
  std::lock_guard<std::mutex> guard(detail::lock());
  std::string const key_str(key);
  return detail::values().contains(key_str) || detail::defaults().contains(key_str);
}

auto Config::get(std::string_view key) -> std::string {
  std::lock_guard<std::mutex> guard(detail::lock());
  std::string const key_str(key);
  auto const& values = detail::values();
  auto const it = values.find(key_str);
  if (it != values.end()) return it->second;
  auto const& defaults = detail::defaults();
  auto const dit = defaults.find(key_str);
  return dit == defaults.end() ? std::string{} : dit->second;
}

auto Config::get(std::string_view key, std::string_view fallback) -> std::string {
  auto const value = get(key);
  return value.empty() ? std::string(fallback) : value;
}

auto Config::get_int(std::string_view key, std::int64_t fallback) -> std::int64_t {
  auto const value = get(key);
  if (value.empty()) return fallback;
  std::int64_t result = fallback;
  auto const [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
  return ec == std::errc{} ? result : fallback;
}

auto Config::get_bool(std::string_view key, bool fallback) -> bool {
  auto const parsed = detail::parse_bool(get(key));
  return parsed.has_value() ? *parsed : fallback;
}

}
