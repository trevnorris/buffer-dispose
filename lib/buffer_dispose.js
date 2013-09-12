var bindings = require('bindings')('buffer_dispose.node')
module.exports = dispose;

function dispose(buf) {
  if (typeof buf !== 'object')
    throw new TypeError('argument must be an object');

  if (buf.parent) {
    bindings.unslice(buf)
    buf.parent = undefined;
  } else {
    bindings.dispose(buf);
  }

  buf.length = 0;
}
