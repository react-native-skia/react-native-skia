
import React, { Component } from 'react';
import { Keyboard, Text, TextInput, StyleSheet, View, AppRegistry } from 'react-native';

class SimpleViewApp  extends Component {
  state = {
    keyboardStatus: undefined
  };

  componentDidMount() {
    this.keyboardDidShowSubscription = Keyboard.addListener(
      'keyboardDidShow',
      () => {
        this.setState({ keyboardStatus: 'Keyboard Shown' });
      },
    );
    this.keyboardDidHideSubscription = Keyboard.addListener(
      'keyboardDidHide',
      () => {
        this.setState({ keyboardStatus: 'Keyboard Hidden' });
      },
    );
  }

  componentWillUnmount() {
    this.keyboardDidShowSubscription.remove();
    this.keyboardDidHideSubscription.remove();
  }

  render() {
    return (
      <View style={style.container}>
        <TextInput
          style={style.input}
          placeholder='Click here…'
          onChangeText={Keyboard.dismiss}
        />
        <Text style={style.status}>
          {this.state.keyboardStatus}
        </Text>
      </View>
    )
  }
}

const style = StyleSheet.create({
  container: {
    flex: 1,
    padding: 36
  },
  input: {
    padding: 10,
    borderWidth: 0.5,
    borderRadius: 4
  },
  status: {
    padding: 10,
    textAlign: "center"
  }
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
