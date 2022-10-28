// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const path = require('path');

const CopyPlugin = require('copy-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const VueLoaderPlugin = require('vue-loader/lib/plugin');

module.exports = {
  // Defines two entry point chunks, one for each page.
  entry: {
    packageView: './src/package_view.js',
    classView: './src/class_view.js',
  },
  // The chunks will be output as packageView.bundle.js, etc.
  output: {
    filename: '[name].bundle.js',
  },
  // An alias so we import from the version of 'vue' which has the template
  // compiler. Not sure if this will be necessary with SFCs.
  resolve: {
    alias: {
      vue: 'vue/dist/vue.js',
    },
  },
  // Generates a source map to help with development.
  // (https://webpack.js.org/configuration/devtool/#devtool)
  devtool: 'inline-source-map',
  // Config options for webpack-dev-server.
  devServer: {
    // Enables hot module replacement
    // (https://webpack.js.org/concepts/hot-module-replacement/).
    hot: true,
    // Opens localhost:8888/package_view.html on dev-server startup.
    port: 8888,
    open: true,
    contentBase: path.resolve(__dirname, 'src'),
    openPage: 'package_view.html',
    // Suppresses most command-line output from dev-server.
    stats: 'minimal',
  },
  optimization: {
    // Splits out common dependencies from all chunks into their own bundles.
    splitChunks: {
      chunks: 'all',
    },
    // Do not minify output.
    minimize: false,
  },
  // Rules for which loaders to use with which filetype.
  module: {
    rules: [
      {
        test: /\.vue$/,
        include: path.resolve(__dirname, 'src/vue_components'),
        use: 'vue-loader',
      },
      {
        test: /\.css$/,
        use: ['vue-style-loader', 'css-loader'],
      },
      {
        test: /\.scss$/,
        use: ['vue-style-loader', 'css-loader', 'sass-loader'],
      },
      {
        test: /\.(png|svg|jpg|gif)$/,
        use: 'file-loader',
      },
    ],
  },
  plugins: [
    // Defines two HTML outputs, one for each page.
    new HtmlWebpackPlugin({
      filename: 'package_view.html',
      template: './src/package_view.html',
      favicon: './src/assets/package_graph_icon.png',
      chunks: ['packageView'],
    }),
    new HtmlWebpackPlugin({
      filename: 'class_view.html',
      template: './src/class_view.html',
      favicon: './src/assets/class_graph_icon.png',
      chunks: ['classView'],
    }),
    // For development purposes: Copies `json_graph.txt` in `src` to `src/dist`
    // on build.
    new CopyPlugin({
      patterns: [
        {
          from: path.resolve(__dirname, 'src/json_graph.txt'),
          to: path.resolve(__dirname, 'dist/json_graph.txt'),
        },
      ],
    }),
    new VueLoaderPlugin(),
  ],
};
