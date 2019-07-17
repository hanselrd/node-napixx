#include <cstdint>
#include <napixx.hpp>
#include <node_api.h>

napi_value my_function(napi_env env, napi_callback_info info) {
  napi_status status = napi_generic_failure;
  std::size_t argc = 1;
  std::int32_t number = 0;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to parse arguments");
  }

  status = napi_get_value_int32(env, argv[0], &number);

  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Invalid number was passed as argument");
  }

  auto js_number = napixx::value{env, number * 2};

  return js_number;
}

napi_value init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value fn;

  status = napi_create_function(env, nullptr, 0, my_function, nullptr, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Unable to wrap native function");
  }

  status = napi_set_named_property(env, exports, "myFunction", fn);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Unable to populate exports");
  }

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init);