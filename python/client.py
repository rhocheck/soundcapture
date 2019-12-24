import time
import requests
import json
import sys
import matplotlib.pyplot as plt
from scipy import signal
import numpy as np
import wave
import threading

# constants
urlMaster = "http://h87:10010"
urlRemote = "http://pi-sensehat:10010"
fileMaster = 'master.wav'
fileRemote = 'remote.wav'

def getTime(url):
  try:
    response = requests.get(url + "/time", timeout=3)
  except requests.exceptions.RequestException as e:
    print(e)
    sys.exit(1)

  if response.status_code != 200:
    print(url + "/time - result: {}".format(response.status_code))
    sys.exit(1)

  ret = json.loads(response.text)
  now = int(ret['message'])
  return now

def getSound(url, filename):
  try:
    response = requests.get(url, timeout=5)
  except requests.exceptions.RequestException as e:
    print(e)
    sys.exit(1)
  audio = open(filename, 'wb')
  audio.write(response.content)

def plotSound(masterAudio, remoteAudio):
  # read wav from master and remote audio
  wavMaster = wave.open(masterAudio, "r")
  sigMaster = wavMaster.readframes(-1)
  sigMaster = np.frombuffer(sigMaster, np.int16)

  wavRemote = wave.open(remoteAudio, "r")
  sigRemote = wavRemote.readframes(-1)
  sigRemote = np.frombuffer(sigRemote, np.int16)

  # make sure both array have the same length
  minLen = len(sigMaster)
  if minLen > len(sigRemote):
    minLen = len(sigRemote)
  sigMaster = sigMaster[:minLen]
  sigRemote = sigRemote[:minLen]

  # plot the graphs
  fig, axs = plt.subplots(2)
  fig.suptitle("Cross-correlation of master and remote audio")
  axs[0].plot(sigMaster)
  axs[0].set_title('Master audio')
  axs[1].plot(sigRemote)
  axs[1].set_title('Remote audio')

  # cross correlate
  xcorr = signal.correlate(sigRemote.astype(float), sigMaster.astype(float))
  shift = (xcorr.argmax() - (len(sigMaster)-1)) * 1000.0 / 44100
  resultStr = "Remote audio shiftet by {:.2f} ms".format(shift)
  plt.figtext(0.5, 0.01, resultStr, wrap=True, fontsize=10, horizontalalignment='center')

  plt.show()


if __name__ == '__main__':
  # start recording in 2s from now. plenty of time to cope with network latency
  # recoding devices are synchronized via ptp
  now = getTime(urlMaster)
  startTime = now + 2000000
  urlSoundMaster = (urlMaster + "/sound?time={}").format(startTime)
  urlSoundRemote = (urlRemote + "/sound?time={}").format(startTime)
 
  threadMaster = threading.Thread(target=getSound, args=(urlSoundMaster, fileMaster))
  threadRemote = threading.Thread(target=getSound, args=(urlSoundRemote, fileRemote))
  threadMaster.start()
  threadRemote.start()
  threadMaster.join()
  threadRemote.join()
 
  plotSound(fileMaster, fileRemote)