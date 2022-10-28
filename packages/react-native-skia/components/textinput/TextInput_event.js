import React, {useRef} from "react";
import { View, StyleSheet,TextInput,AppRegistry,Pressable,TouchableOpacity,Text} from "react-native";

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
  const ref_input1 = useRef();
  const ref_input2 = useRef();
  const ref_input3 = useRef();
  const [text, onChangeText] = React.useState("Useless Text");
    return (
      <View style={styles.container}>
      <TouchableOpacity
        style={styles.button}
        hasTVPreferredFocus={true}
        onPress={() => {console.log("is Focused(true 1, false 0)",ref_input1.current.isFocused());ref_input1.current.focus(); console.log("calling textinput 1")} }
        >
        <Text>Press Here</Text>
      </TouchableOpacity>
       <TextInput
       placeholderTextColor="red"
       //value = "immmutable value"
       autoFocus={true}
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
       style={styles.input}
       underlineColorAndroid={"red"}
       clearTextOnFocus={true}
       //caretHidden={true}
       />

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
       caretHidden={true}
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
       
        <TextInput
        placeholder="Input1"
        returnKeyType="next"
        onFocus={()=>{console.log("in First on Focus")}}
        onBlur={()=> {console.log("in First on Blur")}}
        onEndEditing={console.log("in First on EndEditing")}
        onSubmitEditing={() => {ref_input2.current.focus(); console.log("calling textinput 2")} }
        ref={ref_input1}
        style={styles.input}
        />
        <TextInput
        placeholder="Input2"
        returnKeyType="next"
        onFocus={()=>{console.log("in second on Focus")}}
        onBlur={() => {console.log("in second on Blur")}}
        onEndEditing={() =>{console.log("in second on EndEditng")}}
        onSubmitEditing={() => {ref_input3.current.focus(); console.log("calling textinput 3")} }
        style={styles.input}
        ref={ref_input2}
        />
        <TextInput
        placeholder="Input3"
        onFocus={()=>{console.log("in Third on Focus")}}
        onEndEditing={() =>{console.log("in third on EndEditng")}}
        onBlur={() =>{console.log("in thir on Blur")}}
        onSubmitEditing={()=>{ ref_input1.current.clear();console.log("clearing textinput 1")}}
        style={styles.input}
        ref={ref_input3}
        />
        <TouchableOpacity
        style={styles.button}
        onPress={() => {console.log("is Focused(true 1, false 0)",ref_input1.current.isFocused());ref_input1.current.focus(); console.log("calling textinput 1")} }
        >
        <Text>Press Here</Text>
        </TouchableOpacity>

      </View>
    );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: "space-between",
    backgroundColor: "darkseagreen",
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
    height: 20,
    margin: 5,
    borderWidth: 1,
    padding: 5  ,
    backgroundColor: "#fff",
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

