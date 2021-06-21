/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

'use strict';

require('react-native/Libraries/Core/InitializeCore');
const React = require('react');
const ReactNative = require('react-native');
const {
  AppRegistry,
  StyleSheet,
  Text,
  View,
} = ReactNative;

// Keep this list in sync with RNTesterIntegrationTests.m
const TESTS = [
  require('./box')
];

TESTS.forEach(
  /* $FlowFixMe(>=0.54.0 site=react_native_fb,react_native_oss) This comment
   * suppresses an error found when Flow v0.54 was deployed. To see the error
   * delete this comment and run Flow. */
  test => AppRegistry.registerComponent(test.displayName, () => test),
);


type Test = any;

class SimpleViewApp extends React.Component<{...}, $FlowFixMeState> {
  state = {
    test: (null: ?Test),
  };

  render() {
    if (this.state.test) {
      return (
        <View>
          {/* $FlowFixMe(>=0.53.0 site=react_native_fb,react_native_oss) This
           * comment suppresses an error when upgrading Flow's support for
           * React. To see the error delete this comment and run Flow. */}
          <this.state.test />
        </View>
      );
    }
    return (
      <View style={styles.container}>
        <Text style={styles.row}>
          Testing
        </Text>
<View >
{TESTS.map(test => [
              this.setState({test})
          ])}
</View>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    backgroundColor: 'white',
    marginTop: 40,
    margin: 15,
  },
  row: {
    padding: 10,
  },
  testName: {
    fontWeight: '500',
  },
  separator: {
    height: 1,
    backgroundColor: '#bbbbbb',
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);


