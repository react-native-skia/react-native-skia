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
       placeholder="This_is_placeholderText"
       placeholderTextColor="red"
       defaultValue="This_is_defaultValue"
       maxLength={6}
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
    input: {
    fontSize:25,
    height: 100,
    margin: 12,
    borderWidth: 5,
    padding: 10,
    backgroundColor: "lightcyan",
    textAlign:"right",
  },

 
});


export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);


