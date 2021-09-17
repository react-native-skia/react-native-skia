import React from "react";
import { View, StyleSheet,TextInput,AppRegistry } from "react-native";

const onKeyPress =(e) =>{
  console.log("event====>"+ e.nativeEvent.key);
}
const  onChange1 = (string) => {
    console.log("onChange1", string.nativeEvent.text);
    console.log("onChange1")
  }
const SimpleViewApp = () => {
    return (
      <View style={styles.container}>
       <TextInput
       placeholder="Name"
       //placeholderTextColor="red"
       onKeyPress={onKeyPress}
       onChange={onChange1}
        style={styles.input}/>
      </View>
    );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "space-between",
    backgroundColor: "#fff",
    padding: 20,
    margin: 10,
  },
  top: {
    flex: 0.3,
    backgroundColor: "grey",
    borderWidth: 5,
    borderTopLeftRadius: 20,
    borderTopRightRadius: 20,
  },
    input: {
    height: 40,
    margin: 12,
    borderWidth: 2,
    padding: 10,
    backgroundColor: "#fff",
  },

 
});


export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

