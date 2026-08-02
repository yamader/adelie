module;

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

module adelie.db;

import :session;
import :types;

namespace adelie::db {
namespace {

auto next_savepoint() -> std::string {
  static std::atomic<std::uint64_t> counter{0};
  return "adelie_" + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
}

}

Value::Value(std::nullptr_t) : data_(std::monostate{}) {}

Value::Value(double value) : data_(value) {}

Value::Value(std::string value) : data_(std::move(value)) {}

Value::Value(std::string_view value) : data_(std::string(value)) {}

Value::Value(char const* value) : data_(std::string(value)) {}

Value::Value(Blob value) : data_(std::move(value)) {}

auto Value::type() const noexcept -> ValueType {
  switch (data_.index()) {
    case 0:
      return ValueType::null;
    case 1:
      return ValueType::integer;
    case 2:
      return ValueType::real;
    case 3:
      return ValueType::text;
    default:
      return ValueType::blob;
  }
}

auto Value::is_null() const noexcept -> bool { return data_.index() == 0; }

auto Value::as_int() const -> std::int64_t {
  if (auto const* value = std::get_if<std::int64_t>(&data_)) return *value;
  throw std::logic_error("adelie.db: value is not an integer");
}

auto Value::as_double() const -> double {
  if (auto const* value = std::get_if<double>(&data_)) return *value;
  throw std::logic_error("adelie.db: value is not a real");
}

auto Value::as_string() const noexcept -> std::string_view {
  if (auto const* value = std::get_if<std::string>(&data_)) return *value;
  return {};
}

auto Value::as_blob() const noexcept -> Blob const& {
  if (auto const* value = std::get_if<Blob>(&data_)) return *value;
  static Blob const empty;
  return empty;
}

Row::Row(std::vector<std::string> names, std::vector<Value> values)
    : names_(std::move(names)), values_(std::move(values)) {}

auto Row::size() const noexcept -> std::size_t { return values_.size(); }

auto Row::has(std::string_view column) const noexcept -> bool {
  for (auto const& name : names_)
    if (name == column) return true;
  return false;
}

auto Row::names() const noexcept -> std::vector<std::string> const& { return names_; }

auto Row::value(std::size_t index) const -> Value const& { return values_.at(index); }

auto Row::value(std::string_view column) const -> Value const& {
  for (std::size_t i = 0; i < names_.size(); ++i)
    if (names_[i] == column) return values_[i];
  throw std::out_of_range("adelie.db: no column named \"" + std::string(column) + '"');
}

auto Row::operator[](std::size_t index) const -> Value const& { return value(index); }

auto Row::operator[](std::string_view column) const -> Value const& { return value(column); }

auto ResultSet::empty() const noexcept -> bool { return rows_.empty(); }

auto ResultSet::size() const noexcept -> std::size_t { return rows_.size(); }

auto ResultSet::row(std::size_t index) const -> Row const& { return rows_.at(index); }

auto ResultSet::operator[](std::size_t index) const -> Row const& { return rows_.at(index); }

auto ResultSet::first() const noexcept -> Row const* { return rows_.empty() ? nullptr : &rows_.front(); }

auto ResultSet::rows() const noexcept -> std::vector<Row> const& { return rows_; }

auto ResultSet::column_names() const -> std::vector<std::string> {
  if (rows_.empty()) return {};
  return rows_.front().names();
}

auto ResultSet::begin() const noexcept -> std::vector<Row>::const_iterator { return rows_.begin(); }

auto ResultSet::end() const noexcept -> std::vector<Row>::const_iterator { return rows_.end(); }

auto ResultSet::add_row(std::vector<std::string> names, std::vector<Value> values) -> void {
  rows_.emplace_back(std::move(names), std::move(values));
}

Transaction::Transaction(Session& session) : session_(&session) {
  if (session_->in_transaction()) {
    savepoint_ = next_savepoint();
    session_->execute("SAVEPOINT " + savepoint_);
  } else {
    session_->begin();
  }
}

Transaction::~Transaction() {
  if (session_ != nullptr && !committed_) {
    try {
      rollback();
    } catch (...) {
    }
  }
}

auto Transaction::active() const noexcept -> bool { return session_ != nullptr && !committed_; }

auto Transaction::commit() -> void {
  if (session_ == nullptr || committed_) return;
  if (savepoint_.empty()) {
    session_->commit();
  } else {
    session_->execute("RELEASE SAVEPOINT " + savepoint_);
  }
  committed_ = true;
}

auto Transaction::rollback() -> void {
  if (session_ == nullptr || committed_) return;
  if (savepoint_.empty()) {
    session_->rollback();
  } else {
    session_->execute("ROLLBACK TO SAVEPOINT " + savepoint_);
    session_->execute("RELEASE SAVEPOINT " + savepoint_);
  }
  committed_ = true;
}

}
