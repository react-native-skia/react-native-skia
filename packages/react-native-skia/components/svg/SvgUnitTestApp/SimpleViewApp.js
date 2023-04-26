import React, {Component} from 'react';
import { View, Image, Text } from 'react-native';
import {Svg, Circle, G, Line, Rect, Ellipse,Use,Path,LinearGradient,TSpan,Stop,Defs,Polygon,Polyline} from 'react-native-svg';

export default class SimpleViewApp extends Component {
  render() {
    return (
      <View
        style={{ flexDirection:'row',
                 flexWrap: 'wrap',
                 marginHorizontal: 10,
                 marginVertical: 5,
                 alignItems: "center",
                 justifyContent: "center",
                 backgroundColor: '#444',
		 borderWidth:5 }}>
        <Svg height="100" width="100" >
          <Rect x="0" y="0" width="100" height="100" fill="red" />
          <Circle cx="50" cy="50" r="30" fill="yellow" />
          <Circle cx="40" cy="40" r="4" fill="black" />
          <Circle cx="60" cy="40" r="4" fill="black" />
          <Path d="M 40 60 A 10 10 0 0 0 60 60" stroke="black" />
       </Svg>
       <Image
          style={{ width: 250, height: 250 }}
          source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
        />
        <Text style={{ color: '#fff', fontSize: 36,
                     textAlign: 'center', marginTop: 32 }}>
          React Native loves Skia</Text>
        <Svg 
  width="130"
  height="130"
  fill="blue"
  stroke="red"
  color="green"
  viewBox="-16 -16 544 544"
>
  <Path
    d="M318.37,85.45L422.53,190.11,158.89,455,54.79,350.38ZM501.56,60.2L455.11,13.53a45.93,45.93,0,0,0-65.11,0L345.51,58.24,449.66,162.9l51.9-52.15A35.8,35.8,0,0,0,501.56,60.2ZM0.29,497.49a11.88,11.88,0,0,0,14.34,14.17l116.06-28.28L26.59,378.72Z"
    strokeWidth="32"
  />
  <Path d="M0,0L512,512" stroke="currentColor" strokeWidth="32" />
</Svg>

      </View>
    );
  }
};
