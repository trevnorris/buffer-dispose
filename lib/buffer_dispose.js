var bindings = require('bindings')('bdispose.node')
module.exports = dispose;

function dispose(buf) {
  if (buf.parent)
    throw new Error('Buffer cannot be a slice');
  bindings.dispose(buf);
  buf.length = 0;
}
