import * as React from 'react';
import { useState } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

var width=400;
var height=300;
var resizetest = [
    require('react-native/Libraries/TestImage/512.png'),
    require('react-native/Libraries/TestImage/snackexpo.png'),
];
var resizemodes = [
    "center",
    "cover",
    "contain",
    "stretch",
    "repeat",
];
var downscale =0;
var upscale =1;
var toTest=upscale;

var useCases =5;
var timerValue=10000;

const SimpleViewApp = React.Node = () => {

const [UseCaseCount, toggleState] = useState(true);
const timer = setTimeout(()=>{
           toggleState((UseCaseCount+1)%useCases);
       }, timerValue)

  return (
    <View
      style={{ flex: 1,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: '#444',
               padding:30,
               margin: 15,
               width : 1280,
               height : 720,
               }}>
      <Image
        style={{width:width,height:height,resizeMode: resizemodes[UseCaseCount],borderColor:"green",borderWidth:0}}
        source={resizetest[toTest]}
      />
      <Text style={{ color: 'red', fontSize: 36,
                     textAlign: 'center', marginTop: 32 }}>
      {resizemodes[UseCaseCount]}</Text>

    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
