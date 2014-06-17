## buffer-dispose

If this is so dangerous why should you use it? Because it can drastically
increase throughput by relieving pressure from the garbage collector.

This is what's left of an attempt to allow users to manually free memory
attached to a Buffer instance. It was too precarious to do in core, so now
I'm making my efforts available here.

-----

**Note:** This code is kept up with latest master. When v0.12 is released a
0.1 branch will be created to maintain stability.

-----

### Usage

First a note must be made that `Buffer` has been rewritten in Node v0.11, and
`SlowBuffer` now has a new use. Where `Buffer` will automatically return a
slice of data from a larger slab `SlowBuffer` will always allocate the exact
amount requested, but `SlowBuffer` now returns a normal instance of `Buffer`
instead of its own type.

For this reason the following example uses `SlowBuffer`. Otherwise
`buffer-dispose` would only remove the reference to the slice it pointed to, but
not free the actual memory.

```javascript
var dispose = require('buffer-dispose');

var SlowBuffer = require('buffer').SlowBuffer;
var buf = new SlowBuffer(5);
console.log(buf);
dispose(buf);
console.log(buf);

// output:
// <Buffer 00 00 00 00 00>
// <Buffer >
```

The best use case is when receiving incoming data. Previously data returned from
an incoming `read()` or `'data'` event was allocated from an internal slab. This
has been removed, and now all incoming allocations are `malloc`'d individually.

While this might seem to be a waste, understand that the overhead of tracking
the slab along with the associated V8 calls necessary to do so, outweigh any
performance benefit possibly gained.

-----

The following is the simplest example of immediately disposing of incoming data.
Remember that all incoming buffers from a connection are discretely allocated,
allowing the data to be immediately `free()`'d.

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

Though **make sure** all requests against the data are complete. This means you
must be aware of any asynchronous events. In the following example a buffer is
queued to be written to disk, but then memory is released before the
asynchronous event is able to finish.

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

Probably a better way to do this, but eh. It works for now. Submit a ticket
if you have something better.

### Performance Tests

To see how using this module can help your I/O I've included a performance
test! It's easy to run (once you have the module built and all). Just run
this:

```
/path/to/build/node ./speed/tcp.js <add "true" here to dispose buffers>
```

The below table shows performance differences cleaning up incoming Buffers at
specific sizes. As we can see, the act of disposing has a performance cost.
While in every case we save on memory usage, if performance is more imperative
then tune your application accordingly.

```
64KB Writes     Throughput   Memory Usage
-----------------------------------------
Sad Ponies       26.2 Gb/s       243.2 MB
Magic Unicorns   42.5 Gb/s        47.6 MB

32KB Writes     Throughput   Memory Usage
-----------------------------------------
Sad Ponies       26.0 Gb/s       243.4 MB
Magic Unicorns   37.6 Gb/s        47.7 MB

16KB Writes     Throughput   Memory Usage
-----------------------------------------
Sad Ponies       25.3 Gb/s      243.5 MB
Magic Unicorns   26.6 Gb/s       47.6 MB
```
