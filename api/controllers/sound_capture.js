'use strict';
/*
 'use strict' is not required but helpful for turning syntactical errors into true errors in the program flow
 https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Strict_mode
*/

var util = require('util');
var addon = require('bindings')('addon.node');

module.exports = {
  time: getTime,
  sound: getSound
};

function getTime(req, res) {
  var strVal = util.format('%d', addon.gettimeofday());
  res.json({message: strVal});
}

function getSound(req, res) {
  var timeStr = req.swagger.params.time.value;
  var sound = req.Sound;
  sound.record(timeStr, (val) => {
    res.contentType('audio/wav')
    res.send(val);
  });
}