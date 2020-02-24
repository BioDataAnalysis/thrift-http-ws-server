const path = require('path');

module.exports = {
  entry: ['./index.js'],
  mode: 'development',
  optimization: {
    minimize: false,
  },
  resolve: {
    extensions: ['.js'],
    alias: {
      '@': path.resolve(__dirname, './'),
      'node_modules': path.resolve(__dirname, './node_modules')
    },
  },
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'js/[name].js',
  },
};