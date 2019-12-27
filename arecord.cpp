#include "arecord.h"

CArecord::CArecord(std::string device)
{
  m_device = device;
}

CArecord::~CArecord()
{
  Dispose();
}

void CArecord::Dispose()
{
  if (m_params)
    snd_pcm_hw_params_free(m_params);

  if (m_handle)
    snd_pcm_close(m_handle);

  if (m_buffer)
    free(m_buffer);

  m_buffer = nullptr;
  m_handle = nullptr;
  m_params = nullptr;
}

bool CArecord::Init()
{
  snd_pcm_uframes_t frames;
  int rc;

  // Open PCM device for recording (capture).
  rc = snd_pcm_open(&m_handle, m_device.c_str(), SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0)
  {
    printf("unable to open pcm device: %s\n", snd_strerror(rc));
    return false;
  }

  // Allocate a hardware parameters object.
  snd_pcm_hw_params_malloc(&m_params);

  // Fill it in with default values
  snd_pcm_hw_params_any(m_handle, m_params);

  // Set the desired hardware parameters

  // Interleaved mode
  snd_pcm_hw_params_set_access(m_handle, m_params, SND_PCM_ACCESS_RW_INTERLEAVED);

  // Signed 16-bit little-endian format
  snd_pcm_hw_params_set_format(m_handle, m_params, SND_PCM_FORMAT_S16_LE);

  // one channel (mono) */
  snd_pcm_hw_params_set_channels(m_handle, m_params, 1);

  // 44100 bits/second sampling rate (CD quality)
  unsigned int val = 44100;
  snd_pcm_hw_params_set_rate_near(m_handle, m_params, &val, 0);

  // Set period size to 32 frames
  frames = 32;
  snd_pcm_hw_params_set_period_size_near(m_handle, m_params, &frames, 0);

  // Write the parameters to the driver
  rc = snd_pcm_hw_params(m_handle, m_params);
  if (rc < 0)
  {
    printf("unable to set hw parameters: %s\n", snd_strerror(rc));
    return false;
  }

  rc = snd_pcm_prepare (m_handle);
  if (rc < 0)
  {
    printf("cannot prepare audio interface for use (%s)\n", snd_strerror(rc));
    return false;
  }

  return true;
}

bool CArecord::Record(unsigned int millis)
{
  snd_pcm_uframes_t frames;
  snd_pcm_hw_params_get_period_size(m_params, &frames, 0);
  unsigned int periodSize = frames * 2; // 2 bytes/sample, 1 channel
  char periodBuffer[periodSize];

  unsigned int val;
  snd_pcm_hw_params_get_period_time(m_params, &val, 0);
  unsigned int loops = millis / (val / 1000.0);

  m_bufferSize = loops * periodSize;
  m_buffer = (char*)realloc(m_buffer, m_bufferSize + 44);

  int rc;
  for (unsigned int i=0; i<loops; i++)
  {
    rc = snd_pcm_readi(m_handle, periodBuffer, frames);
    if (rc == -EPIPE)
    {
      // EPIPE means overrun */
      printf("overrun occurred\n");
      snd_pcm_prepare(m_handle);
    }
    else if (rc < 0)
    {
      printf("error from read: %s\n", snd_strerror(rc));
      return false;
    }
    else if (rc != (int)frames)
    {
      printf("short read, read %d frames\n", rc);
      return false;
    }
    memcpy(m_buffer+44+i*periodSize, periodBuffer, periodSize);
  }
  return true;
}

static void set16le(void *const ptr, const unsigned int value)
{
    unsigned char *data = (unsigned char *)ptr;
    data[0] = value & 255U;
    data[1] = (value >> 8) & 255U;
}

static void set32le(void *const ptr, const unsigned int value)
{
    unsigned char *data = (unsigned char *)ptr;
    data[0] = value & 255U;
    data[1] = (value >> 8) & 255U;
    data[2] = (value >> 16) & 255U;
    data[3] = (value >> 24) & 255U;
}

char* CArecord::GenerateHeader()
{
  const int samplesperchannel = m_bufferSize / 2;
  const int channels = 1;
  const int rate = 44100;

  m_buffer[0] = 'R';
  m_buffer[1] = 'I';
  m_buffer[2] = 'F';
  m_buffer[3] = 'F';
  set32le(m_buffer + 4, 36U + 2U*channels*samplesperchannel);
  m_buffer[8] = 'W';
  m_buffer[9] = 'A';
  m_buffer[10] = 'V';
  m_buffer[11] = 'E';
  m_buffer[12] = 'f';
  m_buffer[13] = 'm';
  m_buffer[14] = 't';
  m_buffer[15] = ' ';
  set32le(m_buffer + 16, 16U);
  set16le(m_buffer + 20, 1U); /* PCM format */
  set16le(m_buffer + 22, channels);
  set32le(m_buffer + 24, rate);
  set32le(m_buffer + 28, rate * 2U * channels);
  set16le(m_buffer + 32, 2U * channels);
  set16le(m_buffer + 34, 16U); /* Bits per sample */
  m_buffer[36] = 'd';
  m_buffer[37] = 'a';
  m_buffer[38] = 't';
  m_buffer[39] = 'a';
  set32le(m_buffer + 40, 2U*channels*samplesperchannel);
  return m_buffer;
}

char* CArecord::GetRawBuffer()
{
  return m_buffer + 44;
}

unsigned int CArecord::GetBufferSize()
{
  return m_bufferSize;
}
