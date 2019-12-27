#pragma once

#include <napi.h>
#include <memory>
#include <thread>

class CArecord;

class CRecordWrap : public Napi::ObjectWrap<CRecordWrap>
{
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  CRecordWrap(const Napi::CallbackInfo& info);
  virtual ~CRecordWrap();

 private:
  static Napi::FunctionReference constructor;

  Napi::Value Record(const Napi::CallbackInfo& info);

  std::unique_ptr<CArecord> m_arecord;
  std::thread m_nativeThread;
  Napi::ThreadSafeFunction m_tsfn;
};
