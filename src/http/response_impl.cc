module;

#include <boost/http/response.hpp>
#include <boost/json.hpp>

module adelie.http;

import std;
import adelie.support.utils;

namespace adelie::http {

using support::boost_view;
using support::view_of;

struct Response::Impl {
  boost::http::response message{Status::ok};
  std::string body;
};

Response::Response() : impl_{std::make_unique<Impl>()} {}

Response::Response(Status status) : impl_{std::make_unique<Impl>()} { impl_->message.set_start_line(status); }

Response::Response(Status status, std::string body) : impl_{std::make_unique<Impl>()} {
  impl_->message.set_start_line(status);
  impl_->body = std::move(body);
}

Response::Response(Response const& other) : impl_{std::make_unique<Impl>(*other.impl_)} {}

Response::Response(Response&& other) noexcept = default;

auto Response::operator=(Response const& other) -> Response& {
  if (this != &other) *impl_ = *other.impl_;
  return *this;
}

auto Response::operator=(Response&& other) noexcept -> Response& = default;

Response::~Response() = default;

auto Response::text(std::string body, Status status) -> Response {
  Response r{status, std::move(body)};
  r.set_header(Field::content_type, "text/plain; charset=utf-8");
  return r;
}

auto Response::html(std::string body, Status status) -> Response {
  Response r{status, std::move(body)};
  r.set_header(Field::content_type, "text/html; charset=utf-8");
  return r;
}

auto Response::json(std::string body, Status status) -> Response {
  Response r{status, std::move(body)};
  r.set_header(Field::content_type, "application/json");
  return r;
}

auto Response::json(std::initializer_list<std::pair<std::string_view, std::string_view>> fields, Status status)
    -> Response {
  boost::json::object object;
  for (auto const& [key, value] : fields) object[boost_view(key)] = boost_view(value);
  return json(boost::json::serialize(object), status);
}

auto Response::no_content() -> Response { return Response{Status::no_content}; }

auto Response::redirect(std::string_view location, Status status) -> Response {
  Response r{status};
  r.set_header(Field::location, location);
  return r;
}

auto Response::status() const noexcept -> Status { return impl_->message.status(); }

auto Response::status(Status value) -> Response& {
  impl_->message.set_start_line(value);
  return *this;
}

auto Response::reason() const noexcept -> std::string_view {
  return view_of(boost::http::to_string(impl_->message.status()));
}

auto Response::header(std::string_view name) const noexcept -> std::string_view {
  return view_of(impl_->message.value_or(boost_view(name), ""));
}

auto Response::header(Field name) const noexcept -> std::string_view {
  return view_of(impl_->message.value_or(name, ""));
}

auto Response::has_header(std::string_view name) const noexcept -> bool {
  return impl_->message.count(boost_view(name)) > 0;
}

auto Response::set_header(std::string_view name, std::string_view value) -> Response& {
  impl_->message.set(boost_view(name), boost_view(value));
  return *this;
}

auto Response::set_header(Field name, std::string_view value) -> Response& {
  impl_->message.set(name, boost_view(value));
  return *this;
}

auto Response::headers() const -> std::vector<Header> {
  std::vector<Header> out;
  for (auto const& f : impl_->message) out.push_back(Header{std::string{f.name}, std::string{f.value}});
  return out;
}

auto Response::body() const noexcept -> std::string_view { return impl_->body; }

auto Response::body(std::string value) -> Response& {
  impl_->body = std::move(value);
  return *this;
}

auto Response::append(std::string_view value) -> Response& {
  impl_->body += value;
  return *this;
}

}
