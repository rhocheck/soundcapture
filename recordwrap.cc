#include "recordwrap.h"
#include "arecord.h"
#include <thread>
#include <sys/time.h>
#include <inttypes.h>
#include <unistd.h>

Napi::FunctionReference CRecordWrap::constructor;

Napi::Object CRecordWrap::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "CRecordWrap",
                  {InstanceMethod("record", &CRecordWrap::Record)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("CRecordWrap", func);
  return exports;
}

CRecordWrap::CRecordWrap(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<CRecordWrap>(info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length <= 0 || !info[0].IsString())
  {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::String str = info[0].As<Napi::String>();
  m_arecord.reset(new CArecord(str));
}

CRecordWrap::~CRecordWrap()
{

}

Napi::Value CRecordWrap::Record(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();
  Napi::Function cb = info[0].As<Napi::Function>();

  std::string startTimeStr = info[1].As<Napi::String>();
  uint64_t startTime = std::stoll(startTimeStr);

  printf("--- start time: %" PRIu64 "\n", startTime);

  // Create a ThreadSafeFunction
  m_tsfn = Napi::ThreadSafeFunction::New(
      env,
      cb,                            // JavaScript function called asynchronously
      "ARecord",                     // Name
      0,                             // Unlimited queue
      1,                             // Only one thread will use this initially
      [this]( Napi::Env ) {          // Finalizer used to clean threads up
        m_nativeThread.join();
      } );

  // Create a native thread
  m_nativeThread = std::thread( [this, startTime] {

    auto callback = [this]( Napi::Env env, Napi::Function jsCallback) {

      char* buf = m_arecord->GenerateHeader();
      int buflen = m_arecord->GetBufferSize() + 44;

      Napi::Buffer<char> ret = Napi::Buffer<char>::Copy(env, buf, buflen);

      m_arecord->Dispose();

      Napi::String error = Napi::String::New(env, "");
      jsCallback.Call({ret, error});
    };

    m_arecord->Init();

    timeval tv;
    gettimeofday(&tv, 0);
    uint64_t now = (uint64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
    uint64_t diff = startTime - now;
    printf("------ sleep: %" PRIu64 "\n", diff);
    usleep(diff);

    m_arecord->Record(2000);

    napi_status status = m_tsfn.BlockingCall(callback);
    if ( status != napi_ok )
    {
      // Handle error
    }

    // Release the thread-safe function
    m_tsfn.Release();
  } );

  return Napi::Boolean::New(env, true);
}
