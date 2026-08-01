module;

#include <boost/beast2/http_server.hpp>
#include <boost/capy/buffers.hpp>
#include <boost/corosio/endpoint.hpp>
#include <boost/corosio/io_context.hpp>
#include <boost/corosio/ipv4_address.hpp>
#include <boost/http/config.hpp>
#include <boost/http/server/route_handler.hpp>
#include <boost/http/server/router.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <regex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

module adelie.server;

import adelie.http;
import adelie.routing.pattern;
import adelie.routing.route;
import adelie.support.utils;

namespace adelie::server {
namespace {

using support::boost_view;
using support::view_of;

struct CompiledConstraint {
  std::string parameter;
  std::regex pattern;
};

auto find_parameter(std::vector<std::pair<std::string, std::string>> const& params, std::string_view name)
    -> std::string const* {
  for (auto const& param : params)
    if (param.first == name) return &param.second;
  return nullptr;
}

auto run_chain(std::span<routing::Middleware const* const> chain, routing::Handler const& handler,
               http::Request& request) -> http::Response {
  if (chain.empty()) return handler(request);
  routing::Next next = [tail = chain.subspan(1), &handler, &request] { return run_chain(tail, handler, request); };
  return (*chain.front())(request, next);
}

auto compile_constraints(routing::RouteDefinition const& route) -> std::vector<CompiledConstraint> {
  std::vector<CompiledConstraint> out;
  for (auto const& constraint : route.constraints())
    out.push_back(CompiledConstraint{constraint.parameter,
                                     std::regex(constraint.pattern, std::regex::ECMAScript | std::regex::optimize)});
  return out;
}

auto resolve_chain(routing::Router const& routes, routing::RouteDefinition const& route)
    -> std::vector<routing::Middleware const*> {
  std::vector<routing::Middleware const*> out;
  for (auto const& name : route.middleware()) {
    auto const* middleware = routes.middleware_for(name);
    if (middleware == nullptr)
      throw std::invalid_argument("adelie: unregistered middleware \"" + name + "\" on route \"" +
                                  std::string(route.uri()) + '"');
    out.push_back(middleware);
  }
  return out;
}

auto make_request(boost::http::route_params& p, std::string name) -> http::Request {
  http::Request request(p.req.method(), view_of(p.req.target()));
  for (auto const& field : p.req) request.set_header(view_of(field.name), view_of(field.value));
  request.set_route_name(std::move(name));
  request.set_route_parameters(p.params);
  return request;
}

auto write_response(boost::http::route_params& p, http::Response const& response) -> void {
  p.res.clear();
  p.res.set_start_line(response.status());
  for (auto const& header : response.headers()) p.res.set(boost_view(header.name), boost_view(header.value));
}

auto guarded(routing::Handler const& handler, std::span<routing::Middleware const* const> chain, http::Request& request)
    -> http::Response {
  try {
    return run_chain(chain, handler, request);
  } catch (std::exception const& e) {
    return http::Response::text(std::string("adelie: ") + e.what(), http::Status::internal_server_error);
  } catch (...) {
    return http::Response::text("adelie: unknown error", http::Status::internal_server_error);
  }
}

auto make_fallback(routing::Router const& routes) {
  return [handler = routes.fallback_handler()](boost::http::route_params& p) -> boost::http::route_task {
    auto request = make_request(p, {});
    auto const response = handler != nullptr ? guarded(*handler, {}, request)
                                             : http::Response::json({{"error", "not found"}, {"path", request.path()}},
                                                                    http::Status::not_found);
    write_response(p, response);
    co_await p.send(response.body());
    co_return boost::http::route_done;
  };
}

auto make_handler(routing::Router const& routes, routing::RouteDefinition const& route) {
  return [handler = &route.handler(), name = std::string(route.name()), constraints = compile_constraints(route),
          chain = resolve_chain(routes, route)](boost::http::route_params& p) -> boost::http::route_task {
    for (auto const& constraint : constraints) {
      auto const* value = find_parameter(p.params, constraint.parameter);
      if (value != nullptr && !std::regex_match(*value, constraint.pattern)) co_return boost::http::route_next_route;
    }

    auto request = make_request(p, name);

    if (p.req_body) {
      std::string body;
      boost::capy::const_buffer chunks[8];
      for (;;) {
        auto [ec, filled] = co_await p.req_body.pull(chunks);
        if (ec) break;
        std::size_t taken = 0;
        for (auto const& chunk : filled) {
          body.append(static_cast<char const*>(chunk.data()), chunk.size());
          taken += chunk.size();
        }
        if (taken == 0) break;
        p.req_body.consume(taken);
      }
      request.set_body(std::move(body));
    }

    auto const response = guarded(*handler, chain, request);
    write_response(p, response);

    co_await p.send(response.body());
    co_return boost::http::route_done;
  };
}

auto build_router(routing::Router const& routes) -> boost::http::router<boost::http::route_params> {
  boost::http::router<boost::http::route_params> out;
  for (auto const& route : routes.routes()) {
    auto const pattern = route.pattern().to_boost_pattern();
    if (route.methods() == http::MethodSet::all()) {
      out.all(pattern, make_handler(routes, route));
      continue;
    }
    for (auto const method : route.methods().to_vector()) out.add(method, pattern, make_handler(routes, route));
  }
  out.use(make_fallback(routes));
  return out;
}

}

struct Server::Impl {
  routing::Router* routes = nullptr;
  std::size_t workers = 1;
  std::string address = "0.0.0.0";
  std::uint16_t port = 8080;
};

Server::Server(routing::Router& routes) : impl_(std::make_unique<Impl>()) { impl_->routes = &routes; }

Server::Server(Server&& other) noexcept = default;

auto Server::operator=(Server&& other) noexcept -> Server& = default;

Server::~Server() = default;

auto Server::workers(std::size_t count) -> Server& {
  impl_->workers = count == 0 ? 1 : count;
  return *this;
}

auto Server::listen(std::string_view address, std::uint16_t port) -> Server& {
  impl_->address = address;
  impl_->port = port;
  return *this;
}

auto Server::address() const noexcept -> std::string_view { return impl_->address; }

auto Server::port() const noexcept -> std::uint16_t { return impl_->port; }

auto Server::run() -> void {
  boost::corosio::ipv4_address addr;
  if (boost::corosio::parse_ipv4_address(impl_->address, addr))
    throw std::invalid_argument("adelie: cannot parse listen address \"" + impl_->address + '"');

  boost::corosio::io_context ctx;
  boost::beast2::http_server server(ctx, impl_->workers, build_router(*impl_->routes),
                                    boost::http::make_parser_config(boost::http::parser_config(true)),
                                    boost::http::make_serializer_config(boost::http::serializer_config{}));

  if (auto const ec = server.bind(boost::corosio::endpoint(addr, impl_->port)))
    throw std::runtime_error("adelie: cannot bind " + impl_->address + ':' + std::to_string(impl_->port) + " - " +
                             ec.message());

  server.start();
  ctx.run();
}

}
