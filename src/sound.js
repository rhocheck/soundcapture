var NanoTimer = require('nanotimer');
var Mic = require('node-mic-record');

function Sound() {

  this.rec = function(cb) {
    var audio = null;
    
    alsaDev = null;
    if (process.env.ALSA_DEVICE) {
      alsaDev = process.env.ALSA_DEVICE;
    }

    var reader = Mic.start({
      sampleRate : 44100,
      channels: 1,
      recordProgram: 'arecord',
      device: alsaDev,
      verbose : true
    });

    reader.on('data', (data) => {
      if (audio == null) {
        audio = new Buffer.from(data);
      } else {
        audio = Buffer.concat([audio, new Buffer.from(data)]);
      }
    });

    reader.on('end', (data) => {
      console.log('Recording end, bytes: %d', audio.length);
      cb(audio);
    });

    setTimeout(() => {
      Mic.stop();
    }, 2000);
  } 
}

Sound.prototype.record = function(startTime, cb) {
   var timeStr = startTime + 'u';
   var timerObject = new NanoTimer();
   timerObject.setTimeout(this.rec, [cb], timeStr);
}

module.exports = Sound;