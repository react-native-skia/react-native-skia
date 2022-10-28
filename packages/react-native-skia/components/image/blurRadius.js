import * as React from 'react';
import { useState } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';
var resizemodes = [
    "center",
    "cover",
    "contain",
    "stretch",
    "repeat",
];
var blurRadius =[0,-1,-3,5,10,50,100,200,2000]
var useCases =9;
var timerValue=3000;

const SimpleViewApp = React.Node = () => {

const [UseCaseCount, toggleState] = useState(0);
  setTimeout(()=>{
     if(UseCaseCount == useCases ) toggleState(0)
     else toggleState(UseCaseCount+1);    
  }, timerValue)

  return (
    <View
      style={{
               alignItems: 'center',
               justifyContent: "center",
               backgroundColor:'pink',
               margin: 15,
               width : 500,
               height : 500,
               
               }}>
      <Image
        style={{width:500,height:250,resizeMode:resizemodes[UseCaseCount],shadowColor:'yellow',shadowRadius:2,shadowOffset:{width:10,height:10},shadowOpacity:1}}
        source={require('./disney-1-logo-png-transparent.png')}
        blurRadius={blurRadius[UseCaseCount]}
      />
      <Text style={{ color: 'black',marginTop:30, fontSize: 36,textAlign: 'center'}}>
         blurRadius = {blurRadius[UseCaseCount]}</Text>
    </View>
  );
};
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);