var addon = require('bindings')('addon.node');

function Sound() {
  alsaDev = "default";
  if (process.env.ALSA_DEVICE) {
    alsaDev = process.env.ALSA_DEVICE;
  }
  this.rec = new addon.CRecordWrap(alsaDev);
}

Sound.prototype.record = function(startTime, cb) {

  this.rec.record((val, error) => {
    if (error) {
      console.log(error);
      cb('');
    } else {
      cb(val);
    }
  }, startTime);
}

module.exports = Sound;