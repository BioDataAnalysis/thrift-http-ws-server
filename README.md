# thrift-http-ws-server

This project should eventually be included in upstream https://github.com/apache/thrift
when the code is sufficiently matured. Currently the software design follows
https://github.com/boostorg/beast more than Apache thrift. But over time this is
expected to change, so that Boost::beast becomes the workhorse under the hood, but
Apache thrift the high-level interface.

## About

Apache thrift is a powerful RPC framework with bindings for many languages. As the
evolution of software goes, not all language bindings provide the same functionality.
Sometimes this may be due to missing language features that make it hard or impossible
to implement functionality. And sometimes this may be due to sheer lack of manpower or
interest.

At the time time of this writing, the Javascript bindings have fallen behind the
Node.js bindings in several ways. Javascript currently features only a single protocol
and a single transport, whereas Node.js features a number of protocols and transports.
A noteworthy example is the [binary protocol](https://github.com/apache/thrift/blob/master/lib/nodejs/lib/thrift/binary_protocol.js)
from Node.js that does not currently have an equivalent in Javascript.

On the positive side, tools like browserify and webpack make it possible to use some
parts of the Node.js implementation in the browser. Why only parts? Because not all
the features of the node client are provided by the web browser. For example, the node
client can support raw TCP connections, which the browser does not support. Therefore
the TCP connection only works in Node.js, independent of webpack/browserify.

## HowTo

To use the Node.js bindings in a browser, a tool like browserify or webpack is
required to bundle the required dependencies and the various source files into one
Javascript source file.

In this demo application, we have used [webpack](https://webpack.js.org/). The
same should be possible with [browserify](http://browserify.org/), however we have
not tried. Feedback would be very welcome.

### Thrift HowTo

From the point of thrift, we need to target a Node.js client. The bindings should be
generated with the target `js:node`:
```
thrift -strict -recurse -gen js:node -o ./src/ tutorial.thrift
```

For the same reason, the Node.js package and Node.js bindings of thrift will be used.
The [thrift.js](https://github.com/apache/thrift/blob/master/lib/nodejs/lib/thrift/thrift.js)
sources itself does not provide and exports. Therefore it can not be used directly
from webpack/browserify. Exports are provided via wrappers, i.e. via
[browser.js](https://github.com/apache/thrift/blob/master/lib/nodejs/lib/thrift/browser.js)
from the thrift module.

browser.js can be imported into a webpack bundled page with its full path. For
example when installing node modules in the local sub-folder `node_modules`,
the browser.js bindings can be imported as
```
var thrift = require('thrift/lib/nodejs/lib/thrift/browser');
```

### Node.js HowTo

The following development dependencies are required:
 1. Apache thrift and its dependencies:
```
npm install --save thrift bufferutil utf-8-validate error-polyfill
```
 2. Webpack for packaging Node.js:
```
npm install --save-dev webpack webpack-cli
```

## License

This project is licensed under the Apache 2.0 License - see the [LICENSE](LICENSE) file
for details

## Acknowledgments

This project would not have been possible without the help of
 * Hirano Satoshi
 * Eugen Kandakov

