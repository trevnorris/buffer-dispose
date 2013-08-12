## buffer-dispose

This is what's left of an attempt to allow users to manually free memory
attached to a Buffer instance. It was too precarious to do in core, so now I'm
making my efforts available here.

Use like so:

```javascript
var dispose = require('buffer_dispose').dispose;

// this can only be used on non-slices, will throw otherwise
var SlowBuffer = require('buffer').SlowBuffer;
var buf = new SlowBuffer(5);
console.log(buf);
dispose(buf);
console.log(buf);

// output:
// <Buffer 00 00 00 00 00>
// <Buffer >
```

It's mainly meant to be used for incoming data. Now that the `SlabAllocator` has
been removed no incoming I/O is directly part of a slice. Though this is
**only** the case when the stream is in **flowing mode**.

Take the following example:

```javascript
var dispose = require('buffer_dispose').dispose;

function onData(chunk) {
  // do some quick operations on the chunk
  dispose(chunk);
}

function onConnection(socket) {
  socket.on('data', onData);
}

require('net').createServer(onConnection).listen(8000);
```

Each incoming chunk in the above case is uniquely allocated and can be disposed
when the operation is complete. Though you **must** make sure all requests
against the data are complete. This means you must be aware of any asynchronous
events. In the following example a buffer is queued to be written to disk, but
then memory is released before the asynchronous event is able to complete.

```javascript
var dispose = require('buffer_dispose').dispose;
var fs = require('fs');
var buf = require('buffer').SlowBuffer(10);
buf.fill('a');

fs.writeFile('test.txt', buf, function() { });

// disposing here means nothing will be written to disk
dispose(buf);
```

So then if this is so dangerous why should you use it? Because it can
drastically increase throughput by relieving pressure from the garbage
collector.
