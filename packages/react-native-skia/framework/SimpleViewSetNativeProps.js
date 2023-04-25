// Test app for verifying setNativeProps()

import React, { Component, useCallback, useRef, useState } from "react";
import {
  AppRegistry,
  StyleSheet,
  TextInput,
  Text,
  TouchableOpacity,
  TouchableHighlight,
  View,
} from 'react-native';

function printProps(x){
  for (let prop in x) {
    if (x.hasOwnProperty(prop)) {
      console.log("prop name: " + prop + " typeof prop:" + typeof(x[prop]))
    }
  }
}

const SetNativePropsApp = () => {

  const inputRef = useRef(null);
  const touchRef1 = useRef(null);
  const touchRef2 = useRef(null)
  const [bg, setBg] = useState("yellow")

  const editText = useCallback(() => {
    inputRef.current.setNativeProps({text: 'Edited Text'});
    touchRef1.current.setNativeProps({style: {backgroundColor: "green"}})
    touchRef2.current.setNativeProps({style: {backgroundColor: "red"}, underlayColor: "blue"})
    setTimeout(()=>{setBg("cyan")}, 3000)
  }, []);

  const onPress1 = () => {}

  return (
    <View style={[styles.container,{backgroundColor: bg}]}>
      <TextInput ref={inputRef} style={styles.input} />
      <TouchableOpacity ref={touchRef2}  onPress={editText} style={{backgroundColor: "cyan"}}>
        <Text>Edit text and change backgroundColor</Text>
      </TouchableOpacity>

      <TouchableHighlight ref={touchRef1}  onPress={onPress1} underlayColor="red" style={{backgroundColor: "grey", padding: 30, width: 250, height: 100, margin: 20}} >
        <Text > "without-setNativeProp!"</Text>
      </TouchableHighlight>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  input: {
    height: 50,
    width: 200,
    marginHorizontal: 20,
    borderWidth: 1,
    borderColor: '#ccc',
  },
})

export default SetNativePropsApp;
AppRegistry.registerComponent('SimpleViewApp', () => SetNativePropsApp);
