'use strict';

var SwaggerExpress = require('swagger-express-mw');
var app = require('express')();
module.exports = app; // for testing

var config = {
  appRoot: __dirname // required config
};

var sound = require('./src/sound');
var Sound = new sound();

app.use(function (req, res, next) {
  req.Sound = Sound;
  next();
});

SwaggerExpress.create(config, function(err, swaggerExpress) {
  if (err) { throw err; }

  // install middleware
  swaggerExpress.register(app);

  var port = process.env.PORT || 10010;
  app.listen(port);

  if (swaggerExpress.runner.swagger.paths['/time']) {
    console.log('try this:\ncurl http://127.0.0.1:' + port + '/time');
  }
  if (swaggerExpress.runner.swagger.paths['/sound']) {
    console.log('try this:\ncurl http://127.0.0.1:' + port + '/sound?time=epoch_in_us');
  }
});
