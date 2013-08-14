## buffer-dispose

If this is so dangerous why should you use it? Because it can drastically
increase throughput by relieving pressure from the garbage collector.

This is what's left of an attempt to allow users to manually free memory
attached to a Buffer instance. It was too precarious to do in core, so now I'm
making my efforts available here.

### Usage

```javascript
var dispose = require('buffer-dispose');

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
var dispose = require('buffer-dispose');

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
var dispose = require('buffer-dispose');
var fs = require('fs');
var buf = require('buffer').SlowBuffer(10);
buf.fill('a');

fs.writeFile('test.txt', buf, function() { });

// disposing here means nothing will be written to disk
dispose(buf);
```

### Building

To install this from a non-globally installed build of master, use the
following:

```
/path/to/build/node `which npm` --nodedir=/path/to/build install buffer-dispose
```

To build the library I do the following:

```
/path/to/build/node `which npm` --nodedir=/path/to/build install njsutil bindings
/path/to/build/node `which node-gyp` rebuild --nodedir=/path/to/build
```

Probably a better way to do this, but eh. It works for now. Submit a ticket if
you have something better.

### Performance Tests

To see how using this module can help your I/O I've included a performance test!
It's easy to run (once you have the module built and all). Just run this:

```
/path/to/build/node ./speed/tcp.js <add "true" here to dispose buffers>
```

Disposing buffers: 20.5 Gb/s

Not disposing: 17.7 Gb/s
