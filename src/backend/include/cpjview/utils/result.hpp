#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace cpjview {

template <class T, class E> class Result {
  static_assert(!std::is_void_v<T> && !std::is_void_v<E>,
                "Error cannot be void");

public:
  static Result success(T t) { return Result{std::move(t)}; }
  static Result failed(E e) { return Result{std::move(e)}; }

  bool ok() const { return m_storage.index() == 0U; }
  bool nok() const { return !ok(); }

  T &get() { return std::get<0>(m_storage); }
  T const &get() const { return std::get<0>(m_storage); }

  E &take_error() { return std::get<1>(m_storage); }
  E const &take_error() const { return std::get<1>(m_storage); }

private:
  std::variant<T, E> m_storage;

  explicit Result(T t) : m_storage(std::move(t)) {}
  explicit Result(E e) : m_storage(std::move(e)) {}
};

template <class T> class Result<T, T> {
  static_assert(!std::is_void_v<T>, "Error cannot be void");

public:
  static Result success(T t) { return Result{std::move(t), true}; }
  static Result failed(T e) { return Result{std::move(e), false}; }

  bool ok() const { return m_success; }
  bool nok() const { return !ok(); }

  T &get() {
    assert(ok());
    return m_storage;
  }
  T const &get() const {
    assert(ok());
    return m_storage;
  }

  T &take_error() {
    assert(nok());
    return m_storage;
  }
  T const &take_error() const {
    assert(nok());
    return m_storage;
  }

private:
  T m_storage;
  bool m_success;

  explicit Result(T t, bool success)
      : m_storage(std::move(t)), m_success(success) {}
};

template <class E> class Result<void, E> {
  static_assert(!std::is_void_v<E>, "Error cannot be void");

public:
  static Result success() { return Result{}; }
  static Result failed(E e) { return Result{std::move(e)}; }

  bool ok() const { return !m_storage.has_value(); }
  bool nok() const { return !ok(); }

  E &take_error() { return m_storage.value(); }
  E const &take_error() const { return m_storage.value(); }

private:
  std::optional<E> m_storage;

  explicit Result() : m_storage() {}
  explicit Result(E e) : m_storage(std::move(e)) {}
};

} // namespace cpjview
