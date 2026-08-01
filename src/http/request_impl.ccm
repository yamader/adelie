module;

#include <boost/http/request.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

module adelie.http;

import adelie.support.utils;

namespace adelie::http {

using support::boost_view;
using support::view_of;

namespace {

using Pairs = std::vector<std::pair<std::string, std::string>>;

auto lookup(Pairs const& pairs, std::string_view name, std::string_view fallback) noexcept -> std::string_view {
  for (auto const& [k, v] : pairs)
    if (k == name) return v;
  return fallback;
}

auto contains(Pairs const& pairs, std::string_view name) noexcept -> bool {
  for (auto const& [k, v] : pairs)
    if (k == name) return true;
  return false;
}

}

struct Request::Impl {
  boost::http::request message{Method::get, "/"};
  std::string body;
  std::vector<std::pair<std::string, std::string>> route_params;
  std::vector<std::pair<std::string, std::string>> query_params;
  std::string route_name;
  std::size_t path_len = 1;

  auto reparse_target() -> void {
    query_params.clear();
    auto const t = message.target();
    auto const parsed = boost::urls::parse_origin_form(boost::core::string_view(t));
    if (!parsed) {
      path_len = t.size();
      return;
    }
    path_len = parsed->encoded_path().size();
    for (auto const& p : parsed->params()) query_params.emplace_back(p.key, p.value);
  }
};

Request::Request() : impl_(std::make_unique<Impl>()) {}

Request::Request(Method method, std::string_view target) : impl_(std::make_unique<Impl>()) {
  impl_->message = boost::http::request(method, boost_view(target));
  impl_->reparse_target();
}

Request::Request(Request const& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}

Request::Request(Request&& other) noexcept = default;

auto Request::operator=(Request const& other) -> Request& {
  if (this != &other) *impl_ = *other.impl_;
  return *this;
}

auto Request::operator=(Request&& other) noexcept -> Request& = default;

Request::~Request() = default;

auto Request::method() const noexcept -> Method { return impl_->message.method(); }

auto Request::is_method(Method m) const noexcept -> bool { return impl_->message.method() == m; }

auto Request::target() const noexcept -> std::string_view { return view_of(impl_->message.target()); }

auto Request::path() const noexcept -> std::string_view { return target().substr(0, impl_->path_len); }

auto Request::header(std::string_view name) const noexcept -> std::string_view {
  return view_of(impl_->message.value_or(boost_view(name), ""));
}

auto Request::header(Field name) const noexcept -> std::string_view {
  return view_of(impl_->message.value_or(name, ""));
}

auto Request::has_header(std::string_view name) const noexcept -> bool {
  return impl_->message.count(boost_view(name)) > 0;
}

auto Request::set_header(std::string_view name, std::string_view value) -> Request& {
  impl_->message.set(boost_view(name), boost_view(value));
  return *this;
}

auto Request::set_header(Field name, std::string_view value) -> Request& {
  impl_->message.set(name, boost_view(value));
  return *this;
}

auto Request::headers() const -> std::vector<Header> {
  std::vector<Header> out;
  for (auto const& f : impl_->message) out.push_back(Header{std::string(f.name), std::string(f.value)});
  return out;
}

auto Request::body() const noexcept -> std::string_view { return impl_->body; }

auto Request::set_body(std::string value) -> Request& {
  impl_->body = std::move(value);
  return *this;
}

auto Request::route(std::string_view name) const noexcept -> std::string_view {
  return lookup(impl_->route_params, name, {});
}

auto Request::route(std::string_view name, std::string_view fallback) const noexcept -> std::string_view {
  return lookup(impl_->route_params, name, fallback);
}

auto Request::has_route(std::string_view name) const noexcept -> bool { return contains(impl_->route_params, name); }

auto Request::route_parameters() const noexcept -> std::vector<std::pair<std::string, std::string>> const& {
  return impl_->route_params;
}

auto Request::set_route_parameters(std::vector<std::pair<std::string, std::string>> params) -> Request& {
  impl_->route_params = std::move(params);
  return *this;
}

auto Request::query(std::string_view name) const noexcept -> std::string_view {
  return lookup(impl_->query_params, name, {});
}

auto Request::query(std::string_view name, std::string_view fallback) const noexcept -> std::string_view {
  return lookup(impl_->query_params, name, fallback);
}

auto Request::has_query(std::string_view name) const noexcept -> bool { return contains(impl_->query_params, name); }

auto Request::query_parameters() const noexcept -> std::vector<std::pair<std::string, std::string>> const& {
  return impl_->query_params;
}

auto Request::route_name() const noexcept -> std::string_view { return impl_->route_name; }

auto Request::set_route_name(std::string name) -> Request& {
  impl_->route_name = std::move(name);
  return *this;
}

}
