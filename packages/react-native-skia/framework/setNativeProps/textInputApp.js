import React, {useState,useRef} from 'react';
import { TextInput, TouchableOpacity, View, AppRegistry, Text} from 'react-native';

const MyScrollView = () => {

  const textInputRef = useRef(null);
  const [t1Highlight, setT1] = useState("blue")
  const [t2Highlight, setT2] = useState("blue")
  const [t3Highlight, setT3] = useState("blue")
  

   const handleButtonClick = () => {
    if (textInputRef.current) {
        textInputRef.current.setNativeProps({
        style: {
          color: 'red',
          borderColor: 'green',
          borderWidth: 4,
        },
        placeholder: 'New Placeholder',
        value: 'New Value',
        text: 'Updated Text',
        selection: { start: 2, end: 4 },
        hasTVPreferredFocus: true,
    });
  }
   };

  setTimeout(()=>{
    if (textInputRef.current) {
        textInputRef.current.setNativeProps({
        style: {
          color: 'red',
          borderColor: 'green',
          borderWidth: 4,
        },
        placeholder: 'New Placeholder',
        value: 'New Value',
        text: 'Updated Text',
        selection: { start: 2, end: 4 },
        hasTVPreferredFocus: true,
      });
    }
  }, 10000)

  return (
    <View >
    <Text>Click on TextInput Box to See SetNativeProps Changes on Text, Value, Selection and hasTVPreferredFocus </Text>
      <TouchableOpacity  onPress={handleButtonClick} style={{width: 200, height: 50, borderWidth: 1, backgroundColor: "yellow", margin: 30}}>
        <Text>Click to focus TextInput1</Text>
        
      </TouchableOpacity>

      <TextInput
        ref={textInputRef}
        hasTVPreferredFocus={false}
        style={{
          width: 150,
          height: 150,
          backgroundColor: 'lightblue',
          borderRadius: 5,
          paddingLeft: 10,
          margin: 35,
        }}
      placeholder="Enter text"
      defaultValue="Default Value1"
      />

      <TextInput
        hasTVPreferredFocus={true}
        style={{
          width: 300,
          height: 40,
          borderWidth: 1,
          paddingLeft: 10,
          margin: 35,
          borderColor: t3Highlight
        }}
      placeholder="Enter text"
      defaultValue="After 10 Sec, will focus TextInput1"
      hasTVPreferredFocus = {true}
      onPressIn={()=> {console.log("-----focus--TI3"); setT3("red")}}
      onPressOut={()=> {console.log("------blur--TI3"); setT3("blue")}}
      />
    </View>
  );
};
export default MyScrollView;
