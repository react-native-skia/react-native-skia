import React from "react";
import { View, AppRegistry, StyleSheet, TextInput , Alert } from "react-native";

var count = 0;
var timer = null;

startTimer = () => {
    timer = setInterval(()=>{
    if(count == 0) { Alert.alert("Timer-Alert-Title0", "Timer-Alert-msg : " + count); }
        else if(count == 1) { Alert.alert("Timer-Alert-title1", "Timer-Alert-msg : " + count); }
        else if(count == 2) { Alert.alert("Timer-Alert-title2", "Timer-Alert-msg : " + count); }
        else if(count == 3) { Alert.alert("Timer-Alert-title3", "Timer-Alert-msg : " + count); }
        else if(count == 4) { Alert.prompt("Timer-Prompt-title4", "Timer-Prompt-msg : " + count); }
        else if(count == 5) { Alert.prompt("Timer-Prompt-title5", "Timer-Prompt-msg : " + count); }
        else if(count == 6) { Alert.prompt("Timer-Prompt-title6", "Timer-Prompt-msg : " + count); }
        else if(count == 7) { console.log("clearInterval!!!!!!!!!!!!!!"); clearInterval(timer); timer = null; }
        count++;
    }, 2500);
}

const SimpleViewApp = () => {

  startTimer();
  const [text, onChangeText] = React.useState("Useless Text");
  return (
    <View>
      <TextInput
        style={styles.input}
        placeholder="useless placeholder"
        autoFocus={true}       
        onFocus ={()=>{console.log("focus 11 ")}}
        onSubmitEditing ={()=>{console.log("onSubmitEditing 1 ")}}
        onEndEditing={()=>{console.log("onEndEditing 1 ")}}
        onBlur={()=>{console.log("onBlur")}}
      />
    </View>
  );
};

const styles = StyleSheet.create({
  input: {
    height: 40,
    margin: 12,
    borderWidth: 1,
    padding: 10,
    backgroundColor: "#fff"
  },
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

