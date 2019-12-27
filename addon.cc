#include <napi.h>
#include <sys/time.h>
#include "recordwrap.h"

Napi::String getTime(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  timeval tv;

  char micros[256];
  gettimeofday(&tv, 0);
  sprintf(micros, "%lld", (uint64_t)tv.tv_sec * 1000000ll + tv.tv_usec);
  return Napi::String::New(env, micros);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  exports = CRecordWrap::Init(env, exports);
  exports.Set("gettimeofday", Napi::Function::New(env, getTime));
  return exports;
}

NODE_API_MODULE(addon, InitAll)
