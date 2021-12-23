import React from "react";
import { View, StyleSheet,TextInput,AppRegistry,Pressable } from "react-native";

const onKeyPress =(e) =>{
  console.log("event====>"+ e.nativeEvent.key);
}
const  onChange = (string) => {
    console.log("onChange", string.nativeEvent);
  }
const  onChangeText1 = (string) => {
    console.log("onChangeText", string);
}
const onSelectionChange = (event) =>{
  console.log("onSelectionChange", event.nativeEvent);
}
const onFocus = (event) =>{
  console.log("onFocus",event.nativeEvent);
}
const onBlur = (event) =>{
  console.log("onBlur");
}
const onEndEditing = (event) =>{
  console.log("onEndEditing")
}
const onContentSizeChange = (event) =>{
  console.log("onContentSizeChange", event.nativeEvent)
}
const onSubmitEditing = (event) =>{
  console.log("onSubmitEditing",event.nativeEvent)
}
const SimpleViewApp = () => {
  const [text, onChangeText] = React.useState("Useless Text");
    return (
      <View style={styles.container}>
       <TextInput
       placeholderTextColor="red"
       //value = "immmutable value"
       value = {text}
       placeholder="PlaceHolder"
       placeholderTextColor="red"
       defaultValue="Hello"
       onFocus={onFocus}
       onBlur={onBlur}
       onKeyPress={onKeyPress}
       onChange={onChange}
       onChangeText={onChangeText}
       onEndEditing={onEndEditing}
       onSubmitEditing={onSubmitEditing}
       onContentSizeChange={onContentSizeChange}
       onSelectionChange={onSelectionChange}
       style={styles.input}/>

       <TextInput
       placeholderTextColor="red"
       value = "immmutable value"
       //value = {text}
       placeholder="PlaceHolder"
       placeholderTextColor="red"
       defaultValue="Hello"
       onFocus={onFocus}
       onBlur={onBlur}
       onKeyPress={onKeyPress}
       onChange={onChange}
       onChangeText={onChangeText1}
       onEndEditing={onEndEditing}
       onSubmitEditing={onSubmitEditing}
       onContentSizeChange={onContentSizeChange}
       onSelectionChange={onSelectionChange}
       style={styles.input}/>

       <TextInput
       placeholderTextColor="red"
       defaultValue="Hello"
       onFocus={onFocus}
       onBlur={onBlur}
       onKeyPress={onKeyPress}
       onChange={onChange}
       onChangeText={onChangeText1}
       onEndEditing={onEndEditing}
       onSubmitEditing={onSubmitEditing}
       onContentSizeChange={onContentSizeChange}
       onSelectionChange={onSelectionChange}
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

