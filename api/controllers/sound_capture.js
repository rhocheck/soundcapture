'use strict';
/*
 'use strict' is not required but helpful for turning syntactical errors into true errors in the program flow
 https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Strict_mode
*/

var util = require('util');
var native = require('bindings')('util.node');

module.exports = {
  time: getTime,
  sound: getSound
};

function getTime(req, res) {
  // variables defined in the Swagger document can be referenced using req.swagger.params.{parameter_name}
  var name = req.swagger.params.name.value || 'stranger';
  var strVal = util.format('%d', native.gettimeofday());

  res.json({message: strVal});
}

function getSound(req, res) {
  var timeStr = req.swagger.params.time.value;
  var time = parseInt(timeStr, 10);
  var now = parseInt(native.gettimeofday());
  var diff = time - now;
  var sound = req.Sound;
  sound.record(diff, (val) => {
    res.contentType('audio/wav')
    res.send(val);
  });
}