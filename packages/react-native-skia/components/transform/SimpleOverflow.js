import React, { useState,useRef} from "react";
import { TVEventHandler, TouchableHighlight, Pressable, findNodeHandle, Text,View,Image} from "react-native";
import { AppRegistry, Dimensions , StyleSheet} from "react-native";

const { width: viewportWidth, height: viewportHeight } = Dimensions.get('window');

function wp (percentage) {
  const value = (percentage * viewportWidth) / 100;
  return Math.round(value);
}

const containerWidth = wp(40);
const parentWidth = Math.round(containerWidth *0.30);
const parentHeight = parentWidth;

const childWidth = Math.round(parentWidth * 0.30);
const childHeight = childWidth;

const childGap = Math.round(parentWidth * 0.30)
const childLeftAlign = (1.25 *childGap) + childWidth;
const childTopAlign = (1.25 *childGap) + childHeight;

const TouchableBlock = (props) => {

  return (
    <View style={[styles.itemStyle, {overflow: props.overflow, top: props.posy, left: props.posx, borderColor: "black", borderWidth:5, height:parentHeight, width:parentWidth}]}>
      <View style={[styles.childItemStyle, {top:-20, backgroundColor: "blue"}]}/>
      <View style={[styles.childItemStyle, {top:childTopAlign, backgroundColor: "green"}]}/>
      <View style={[styles.childItemStyle, {left:childLeftAlign, backgroundColor: props.bgColor2}]}/>
      <View style={[styles.childItemStyle, {left:childLeftAlign+10, top:childTopAlign+10, backgroundColor: props.bgColor1}]}/>
    </View>
  );
}

const SampleViewPage = () => {
    
    let usrRef = useRef();
    let pswdRef = useRef();
    let [state, setState] = useState({
      bgColor1: "red",
      bgColor2: "yellow"
    });

    const updateChild = () => {
      setState({bgColor1: "yellow", bgColor2: "red"})
    };

    const ViewPage = () => {
      return (
          <View style={[styles.backgroundimage, styles.row]}>
            <View
              style={[styles.itemContainer, {shadowColor: "#000"}]}
              >
                <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} overflow={"hidden"} posx={20} posy={50} ></TouchableBlock>
                <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} overflow={"hidden"} posx={parentWidth+childGap} posy={-parentHeight+50} ></TouchableBlock>
                <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} posx={10} posy={-childHeight} ></TouchableBlock>
                <Text style={styles.title}> *** ACTUAL RESULT ***</Text>
            </View>
            <View
              style={[styles.itemContainer]}
              >
              <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} overflow={"hidden"} posx={20} posy={50} ></TouchableBlock>
              <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} overflow={"hidden"} posx={parentWidth+childGap} posy={-parentHeight+50} ></TouchableBlock>
              <TouchableBlock bgColor1={state.bgColor1} bgColor2={state.bgColor2} posx={10} posy={-childHeight} ></TouchableBlock>
                <Text style={styles.title}> *** EXPECTED RESULT ***</Text>
            </View>
          </View>
      )
    };
  
    setTimeout(updateChild, 2000)
    return (
       ViewPage()
    );
}

const styles = StyleSheet.create({
    backgroundimage: {
       flex: 1,
       alignItems : 'flex-start',
       justifyContent : 'flex-start',
       width: viewportWidth,
       height: viewportHeight,
       backgroundColor: 'white',
       margin:20,
       borderWidth:5,
       borderColor:"blue",
       borderheight: 340,
    },
    itemStyle : {
       backgroundColor: 'lightgrey',	    
    },
    row: {
      flexDirection: 'row',
      flexWrap: 'wrap',
    },
    childItemStyle: {
      position:"absolute",
      height:childWidth,
      width:childWidth,
    },
    itemContainer: {
      margin: 12,
      width: containerWidth,
      alignItems: "flex-start",
      backgroundColor:"grey",
      borderColor:"black",
      borderWidth:5,
    },
    title: {
      color: "white",
      fontSize: 18,
      fontWeight: 'bold',
      letterSpacing: 0.5
    },
});

export default SampleViewPage
AppRegistry.registerComponent('SimpleViewApp', () => SampleViewPage);
