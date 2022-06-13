import React, {useRef} from "react";
import { View,TouchableOpacity,Text,AppRegistry, StyleSheet, TextInput } from "react-native";

const SimpleViewApp = () => {
  const [text, onChangeText] = React.useState("Useless Text");
  const [number, onChangeNumber] = React.useState(null);
  const ref_input1 = useRef();
  const ref_input2 = useRef();
  return (
    <View>
    <TouchableOpacity
        style={styles.button}
        hasTVPreferredFocus={true}
        onPress={() => {console.log("is Focused(true 1, false 0)",ref_input1.current.isFocused());ref_input1.current.focus(); console.log("calling textinput 1")} }
        >
        <Text>Press Here</Text>
        </TouchableOpacity>
      <TextInput
        style={styles.input}
        onChangeText={onChangeText}
        value={text}
        autoFocus={true}
        onFocus ={()=>{console.log("focus 11 ")}}
        onSubmitEditing ={()=>{console.log("onSubmitEditing 1 ")}}
        onEndEditing={()=>{console.log("onEndEditing 1 ")}}
        onBlur={()=>{console.log("onBlur")}}
        ref={ref_input1}
      />
      <TextInput
        style={styles.input}
        value={number}
        placeholder="useless placeholder"
        keyboardType="numeric"
        autoFocus={true}
	onChangeText={()=>{console.log("In second");ref_input1.current.focus();}}
       ref={ref_input2}
      />
      <TouchableOpacity
        style={styles.button}
        //hasTVPreferredFocus={true}
        onPress={() => {console.log("is Focused(true 1, false 0)",ref_input1.current.isFocused());ref_input1.current.focus(); console.log("calling textinput 1")} }
        >
        <Text>Press Here</Text>
        </TouchableOpacity>
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
  button: {
    alignItems: "center",
    backgroundColor: "#DDDDDD",
    padding: 10,
    width: 100
  },
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
