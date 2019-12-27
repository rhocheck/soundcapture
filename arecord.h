#pragma once

#include <string>
#include <alsa/asoundlib.h>

class CArecord
{
public:
  CArecord(std::string device);
  virtual ~CArecord();

  bool Init();
  void Dispose();
  bool Record(unsigned int millis);
  char* GenerateHeader();
  char* GetRawBuffer();
  unsigned int GetBufferSize();

protected:
  snd_pcm_t* m_handle = nullptr;
  snd_pcm_hw_params_t* m_params = nullptr;
  std::string m_device = "default";
  char* m_buffer = nullptr;
  unsigned int m_bufferSize = 0;
};
