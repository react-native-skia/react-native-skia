/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 * Copyright (C) 1994-2023 OpenTV, Inc. and Nagravision S.A.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

import React, { useState,useRef} from "react";
import { TVEventHandler, TouchableHighlight, TouchableOpacity, Pressable, findNodeHandle, TextInput, View, Image, Text} from "react-native";
import { AppRegistry,Dimensions , StyleSheet} from "react-native";

exports.framework = 'React';
exports.title = 'DirectionalNextFocus example';
exports.description = 'tvOS nextFocus';
exports.examples = [
  {
    title: 'DirectionalNextFocus',
    render(): React.Node {
      return <DirectionalNextFocusExample />;
    },
  },
];

const padding = 100;
const width = 200;
const height = 120;

class Button extends React.Component<$FlowFixMeProps> {
  render() {
    return (
            <TouchableOpacity
              nextFocusUp={this.props.nextFocusUp}
              nextFocusDown={this.props.nextFocusDown}
              nextFocusLeft={this.props.nextFocusLeft}
              nextFocusRight={this.props.nextFocusRight}
              activeOpacity={0.7}
              onPress={this.props.onPress}
              onFocus={this.props.onFocus}
              style={[{backgroundColor: "red"}, this.props.style]}
              ref={this.props.ref}>
              <Text style={[{color: "black"}, styles.buttonText]}>
                {this.props.label}
              </Text>
            </TouchableOpacity>
    );
  }
}

class DirectionalNextFocusExample extends React.Component<
  $FlowFixMeProps,
  {
    destinations: {
      up: ?Object,
      down: ?Object,
      left: ?Object,
      right: ?Object,
    },
  },
> {
  constructor(props: Object) {
    super(props);
    this.state = {
      destinations: {
        up: undefined,
        down: undefined,
        left: undefined,
        right: undefined,
      },
    };
  }

  render() {
    const {destinations} = this.state;
    return (
      <View style={styles.container}>
        <View style={styles.rowContainer}>
          <Button
            ref={component => {
              if (!this.state.destinations.right) {
                this.setState(prevState => ({
                  destinations: {
                    ...prevState.destinations,
                    right: findNodeHandle(component),
                  },
                }));
              }
            }}
            style={{
              width,
              height,
            }}
            label="Unwrapped button 1"
          />
          </View>
        <View style={styles.rowContainer}>
          <Button
            nextFocusUp={destinations.up}
            nextFocusDown={destinations.down}
            nextFocusLeft={destinations.left}
            nextFocusRight={destinations.right}
            style={{
              width,
              height,
            }}
            label="Starting point"
          />
          <Button
            ref={component => {
              if (!this.state.destinations.up) {
                this.setState(prevState => ({
                  destinations: {
                    ...prevState.destinations,
                    up: findNodeHandle(component),
                  },
                }));
              }
            }}
            style={{
              width,
              height,
            }}
            label="nextUp destination"
          />
          <View style={styles.containerFocusGuide}>
            <Button
              style={{
                width,
                height,
              }}
              label="Wrapped button 1"
            />
            <Button
              ref={component => {
                if (!this.state.destinations.down) {
                  this.setState(prevState => ({
                    destinations: {
                      ...prevState.destinations,
                      down: findNodeHandle(component),
                    },
                  }));
                }
              }}
              style={{
                width,
                height,
              }}
              label="nextDown destination"
            />
            <Button
              style={{
                width,
                height,
              }}
              label="Wrapped button 3"
            />
          </View>
        </View>
        <View style={styles.rowContainer}>
          <Button
            ref={component => {
              if (!this.state.destinations.right) {
                this.setState(prevState => ({
                  destinations: {
                    ...prevState.destinations,
                    right: findNodeHandle(component),
                  },
                }));
              }
            }}
            style={{
              width,
              height,
            }}
            label="nextRight destination"
          />
          <View
            style={{
              width,
              height,
            }}
          />
          <Button
            ref={component => {
              if (!this.state.destinations.left) {
                this.setState(prevState => ({
                  destinations: {
                    ...prevState.destinations,
                    left: findNodeHandle(component),
                  },
                }));
              }
            }}
            style={{
              width: width * 3,
              height,
            }}
            label="nextLeft destination does not work because there is no actual focusable in the direction"
          />
        </View>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    marginTop: -20,
    backgroundColor: 'transparent',
  },
  rowContainer: {
    flexDirection: 'row',
    padding,
  },
  buttonText: {
    fontSize: 30,
  },
  focusGuide: {
    width,
    height,
    backgroundColor: 'pink',
    opacity: 0.3,
  },
  containerFocusGuide: {
    backgroundColor: 'transparent',
    borderColor: 'blue',
    borderWidth: 2,
    flexDirection: 'row',
  },
});

export default DirectionalNextFocusExample
AppRegistry.registerComponent('SimpleViewApp', () => DirectionalNextFocusExample);
