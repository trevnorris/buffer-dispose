var net = require('net');
var cluster = require('cluster');
var dispose = require('../lib/buffer_dispose');

if (!cluster.isMaster) {
  var b = require('buffer').SlowBuffer(1 << 16);

  var client = net.connect(8123, function() {
    client.on('drain', writeData);
    writeData();
  });

  function writeData() {
    while (client.write(b));
  }

  return;
}


var cntr = 0;
var shouldDispose = !!process.argv[2];
net.createServer(function(c) {
  c.on('data', function(d) {
    cntr += d.length;
    if (shouldDispose)
      dispose(d);
  });
}).listen(8123, function() {
  if (shouldDispose)
    console.error('Your unicorns are disposing the buffers');
  else
    console.error('Sent your buffer disposing unicorns to the glue factory :(');
  console.error('\nlistening on port 8123');
  cluster.fork();
});

var t = process.hrtime();
setInterval(function() {
  t = process.hrtime(t);
  console.error(((cntr / 1024 / 1024 / 1024 * 8) /
                 (t[0] + t[1] / 1e9)).toFixed(2) +
                 ' Gb/s');
  t = process.hrtime();
  cntr = 0;
}, 1000);
