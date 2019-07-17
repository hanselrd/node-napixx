#ifndef NAPIXX_DETAIL_VALUE_HPP
#define NAPIXX_DETAIL_VALUE_HPP

#include <cstddef>
#include <cstdint>
#include <node_api.h>
#include <string>
#include <type_traits>

namespace napixx {
// TODO: add napi_status checking
class value {
public:
  // napi_value
  explicit value(napi_env env, napi_value val) : _env{env}, _val{val} {};

  // null/undefined
  template <class T, class U = std::remove_cv_t<std::remove_reference_t<T>>,
            std::enable_if_t<std::is_same_v<U, std::nullptr_t>> * = nullptr>
  explicit value(napi_env env, T &&val, bool undefined) : _env{env} {
    if (undefined) {
      napi_get_undefined(env, &_val);
    } else {
      napi_get_null(env, &_val);
    }
  }

  // arithmetic (boolean/number)
  template <class T, class U = std::remove_cv_t<std::remove_reference_t<T>>,
            std::enable_if_t<std::is_arithmetic_v<U>> * = nullptr>
  explicit value(napi_env env, T &&val) : _env{env} {
    if constexpr (std::is_same_v<U, bool>) {
      napi_get_boolean(env, val, &_val);
    } else if constexpr (std::is_same_v<U, float> ||
                         std::is_same_v<U, double>) {
      napi_create_double(env, val, &_val);
    } else if constexpr (std::is_same_v<U, std::int8_t> ||
                         std::is_same_v<U, std::int16_t> ||
                         std::is_same_v<U, std::int32_t>) {
      napi_create_int32(env, val, &_val);
    } else if constexpr (std::is_same_v<U, std::uint8_t> ||
                         std::is_same_v<U, std::uint16_t> ||
                         std::is_same_v<U, std::uint32_t>) {
      napi_create_uint32(env, val, &_val);
    } else if constexpr (std::is_same_v<U, std::int64_t>) {
      napi_create_int64(env, val, &_val);
#ifdef NAPI_EXPERIMENTAL
    } else if constexpr (std::is_same_v<U, std::uint64_t>) {
      napi_create_bigint_uint64(env, val, &_val);
#endif
    } else {
      napi_get_undefined(env, &_val);
    }
  }

  // std::string/std::u16string (string)
  template <class T, class U = std::remove_cv_t<std::remove_reference_t<T>>,
            std::enable_if_t<std::is_same_v<U, std::string> ||
                             std::is_same_v<U, std::u16string>> * = nullptr>
  explicit value(napi_env env, T &&val) : _env{env} {
    if constexpr (std::is_same_v<U, std::string>) {
      napi_create_string_utf8(env, val.c_str(), val.length(), &_val);
    } else if constexpr (std::is_same_v<U, std::u16string>) {
      napi_create_string_utf16(env, val.c_str(), val.length(), &_val);
    } else {
      napi_get_undefined(env, &_val);
    }
  }

  // char[N]/char16_t[N] (string)
  template <
      class T, std::size_t N,
      class U = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<
          std::remove_cv_t<std::remove_reference_t<std::decay_t<T>>>>>>,
      std::enable_if_t<std::is_array_v<T[N]> &&
                       (std::is_same_v<U, char> || std::is_same_v<U, char16_t>)>
          * = nullptr>
  explicit value(napi_env env, T (&val)[N]) : _env{env} {
    if constexpr (std::is_same_v<U, char>) {
      napi_create_string_utf8(env, val, N, &_val);
    } else if constexpr (std::is_same_v<U, char16_t>) {
      napi_create_string_utf16(env, val, N, &_val);
    } else {
      napi_get_undefined(env, &_val);
    }
  }

  // char */char16_t * (string)
  template <
      class T,
      class U = std::remove_cv_t<std::remove_reference_t<
          std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>>,
      std::enable_if_t<std::is_pointer_v<T> &&
                       (std::is_same_v<U, char> || std::is_same_v<U, char16_t>)>
          * = nullptr>
  explicit value(napi_env env, T val) : _env{env} {
    if constexpr (std::is_same_v<U, char>) {
      auto str = std::string{val};
      napi_create_string_utf8(env, str.c_str(), str.length(), &_val);
    } else if constexpr (std::is_same_v<U, char16_t>) {
      auto str = std::u16string{val};
      napi_create_string_utf16(env, str.c_str(), str.length(), &_val);
    } else {
      napi_get_undefined(env, &_val);
    }
  }

  // T[N] (array)
  template <
      class T, std::size_t N,
      class U = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<
          std::remove_cv_t<std::remove_reference_t<std::decay_t<T>>>>>>,
      std::enable_if_t<std::is_array_v<T[N]> && (!std::is_same_v<U, char> &&
                                                 !std::is_same_v<U, char16_t>)>
          * = nullptr>
  explicit value(napi_env env, T (&val)[N]) : _env{env} {
    napi_create_array_with_length(env, N, &_val);
    for (auto i = std::size_t{0}; i < N; ++i) {
      napi_set_element(env, _val, i, value{val[i]});
    }
  }

  // T * (array)
  template <
      class T,
      class U = std::remove_cv_t<std::remove_reference_t<
          std::remove_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>>,
      std::enable_if_t<std::is_pointer_v<T> && (!std::is_same_v<U, char> &&
                                                !std::is_same_v<U, char16_t>)>
          * = nullptr>
  explicit value(napi_env env, T val, std::size_t len) : _env{env} {
    napi_create_array_with_length(env, len, &_val);
    for (auto i = std::size_t{0}; i < len; ++i) {
      napi_set_element(env, _val, i, value{val[i]});
    }
  }

  napi_valuetype get_type() const {
    auto result = napi_valuetype{};
    napi_typeof(_env, _val, &result);
    return result;
  }

  napi_env get_env() const { return _env; }

  operator napi_value() const { return _val; }

private:
  napi_env _env;
  napi_value _val;
};
} // namespace napixx

#endif