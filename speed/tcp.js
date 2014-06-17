var net = require('net');
var cluster = require('cluster');
var dispose = require('../lib/buffer_dispose');
var SIZE = 1 << 16;

if (!cluster.isMaster) {
  var b = require('buffer').SlowBuffer(SIZE);

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
    console.error('Unicorns dispatched to disposed of buffers');
  else
    console.error('All unicorns have been sent to the glue factory');
  console.error('\nListening on port 8123. Sending %s bytes of data.', SIZE);
  cluster.fork();
});

// I don't trust setInternval's timing mechanism.
var t = process.hrtime();

setInterval(function() {
  t = process.hrtime(t);
  console.error(((cntr / 1024 / 1024 / 1024 * 8) /
                 (t[0] + t[1] / 1e9)).toFixed(2) +
                 ' Gb/s');
  t = process.hrtime();
  cntr = 0;
}, 1000);
