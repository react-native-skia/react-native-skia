import React, { Component } from 'react';
import { Animated, View, StyleSheet, TouchableOpacity, Text } from 'react-native';

class Myanimated extends Component {
  constructor() {
    super();
    this.animatedValue = new Animated.Value(1);
    this.boxRef = React.createRef();
  }

  handlePress1 = () => {
    this.boxRef.current.setNativeProps({
      style: {
        opacity: 0.5,
      },
    });
  };

  handlePress2 = () => {
    this.boxRef.current.setNativeProps({
      style: {
        transform: [{ scale: 0.6 }],
      },
    });
  };

  render() {
    return (
      <View style={styles.container}>
        <Animated.View ref={this.boxRef} style={[styles.box]}>
        </Animated.View>
        <Text>Click to changes setNativeProps </Text>
        <TouchableOpacity style={styles.button} onPress={this.handlePress1}>
          <Text style={styles.buttonText}>Opacity </Text>
        </TouchableOpacity>

        <TouchableOpacity style={styles.button} onPress={this.handlePress2}>
          <Text style={styles.buttonText}>Transform</Text>
        </TouchableOpacity>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    marginTop: 200,
    justifyContent: 'center',
    alignItems: 'center',
  },
  box: {
    width: 200,
    height: 200,
    backgroundColor: 'red',
  },
  button: {
    marginTop: 20,
    padding: 10,
    backgroundColor: 'blue',
    borderRadius: 5,
    width: 100,
    height: 60,
  },
  buttonText: {
    color: 'white',
    fontWeight: 'bold',
    textAlign: 'center',
  },
});

export default Myanimated;

