import React, {useRef, useState, useEffect} from 'react';
import { View,TouchableOpacity, TouchableHighlight, Text, StyleSheet, Button, findNodeHandle } from 'react-native';

const useNodeHandle_ = (ref) => {
  const [nodeHandle, setNodeHandle] = useState(null);

  useEffect(() => {
    console.log("-----------------xxxxxxxxxxxxxxxxxxxxxxxxx")
    if (ref.current) {
      setNodeHandle(findNodeHandle(ref.current));
    }
  }, [ref.current]);

  return nodeHandle;
};

const MyComponent = () => {
  const ref0 = useRef(null);
  const ref00 = useRef(null);
  const ref1 = useRef(null);
  const ref2 = useRef(null);
  const ref3 = useRef(null);
  const ref4 = useRef(null);

  const nodeRef0 = useNodeHandle_(ref0)
  const nodeRef00 = useNodeHandle_(ref00)
  const nodeRef1 = useNodeHandle_(ref1)
  const nodeRef2 = useNodeHandle_(ref2)
  const nodeRef3 = useNodeHandle_(ref3)
  const nodeRef4 = useNodeHandle_(ref4)


  // nextFocusUp={ref1.current?.props.nativeID} // ID of the component to focus when navigating up
  // nextFocusDown={ref2.current?.props.nativeID} // ID of the component to focus when navigating down
  // nextFocusLeft={ref3.current?.props.nativeID} // ID of the component to focus when navigating left
  // nextFocusRight={ref4.current?.props.nativeID} // ID of the component to focus when navigating right

  return (
    <View style={{flexWrap: "wrap"}}>
      <Text style={{fontSize: 20}}> SET NATIVE PROPS : nextFocus* on TouchableOpacity </Text>


      
        <View style={{flexDirection: "row", padding: 5, flexWrap: "wrap", width: 280, borderWidth: 1, justifyContent: "center", marginTop: 100, marginLeft: 10}}>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button1</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button2</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button3</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button4</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "orange", margin: 10}}
            ref={ref0}
            
            nextFocusUp={nodeRef1}
            nextFocusDown={nodeRef2}
            nextFocusLeft={nodeRef3}
            nextFocusRight={nodeRef4}
          >
            <Text>Start Here</Text>
          </TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button6</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button7</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button8</Text></TouchableOpacity>
          <TouchableOpacity style={{width: 65, height: 30, backgroundColor: "lightgrey", margin: 10}}><Text>Button9</Text></TouchableOpacity>


        </View>      
      


        <View
          ref={ref00}
          style={{ backgroundColor: 'lightpink', padding: 10, marginTop: 30, width: 250, height: 250, left: 350, top: 10, position: "absolute"}}
        > 
          <Text style={styles.text}>UP will focus ITEM1</Text>
          <Text style={styles.text}>DOWN will focus ITEM2</Text>
          <Text style={styles.text}>LEFT will focus ITEM3</Text>
          <Text style={styles.text}>RIGHT will focus ITEM4</Text>
        </View>
        

      <View style={{flexDirection: "row", padding: 20}}>
      
      <TouchableOpacity
        ref={ref1}
        style={[styles.container]}
        onPress={()=>{console.log("---------nativeID:" + Object.keys( ref1.current))}}
        onFocus={()=>{console.log("----------focus item1")}}
      >
        <Text style={styles.text}>ITEM1</Text>
      </TouchableOpacity>
      <TouchableOpacity
        ref={ref2}
        style={[styles.container]}
        onFocus={()=>{console.log("----------focus item2")}}
      >
        <Text style={styles.text}>ITEM2</Text>
      </TouchableOpacity>
      <TouchableOpacity
        ref={ref3}
        style={[styles.container]}
        onFocus={()=>{console.log("----------focus item3")}}
      >
        <Text style={styles.text}>ITEM3</Text>
      </TouchableOpacity>
      <TouchableOpacity
        ref={ref4}
        style={[styles.container]}
        onFocus={()=>{console.log("----------focus item4")}}
      >
        <Text style={styles.text}>ITEM4</Text>
      </TouchableOpacity>
      </View>

      <View style={styles.button}>
      <Button style={styles.button} title={"Press To Change UP Key to ITEM2"}
        color={"lightgreen"}
        onPress={()=>{
            console.log("-----------button pressed------------");
            ref0.current.setNativeProps({nextFocusUp: nodeRef2})
          }}
      />
      </View>
      <View style={styles.button}>
      <Button style={styles.button} title={"Press To Change DOWN Key to ITEM4"} 
        color={"lightblue"}
        onPress={()=>{
            console.log("-----------button pressed------------");
            ref0.current.setNativeProps({nextFocusDown: nodeRef4})
          }}
      />
      </View>
    </View>
  );
};




const styles = StyleSheet.create({
  root: {
    margin: 10,
    width: 1280,
    height: 720
  },
  button: {
    margin: 5,
    borderWidth: 1,
    width: 400
  },
  text:{
    color: "blue",
    margin: 5,
    padding: 10,
    borderWidth: 1,
    fontSize: 16
  },
  container: {
    width: 100,
    height: 60,
    borderWidth: 1,
    margin: 10,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: "lightblue"
  }
});
export default MyComponent;
