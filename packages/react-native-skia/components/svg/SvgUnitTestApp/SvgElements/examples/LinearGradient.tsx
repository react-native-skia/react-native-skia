import React, {Component} from 'react';
import {View} from 'react-native';
import {
  Svg,
  Circle,
  Ellipse,
  Text,
  Rect,
  Defs,
  LinearGradient,
  RadialGradient,
  Stop,
} from 'react-native-svg';

class LinearGradienttest extends Component {
  static title =
    'Horizontal vs Vertical linear gradient from yellow to red';
  render() {
    return (
      <View
        style={{
          width: 600,
          height: 200,
          flexDirection: 'row',
          justifyContent: 'space-around',
        }}>

      <Svg height="150" width="300">
        <Defs>
          <LinearGradient id="grad-3" x1="0%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="0" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-3)" />
      </Svg>
      <Svg height="150" width="300">
        <Defs>
          <LinearGradient id="grad-3" x1="0%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="0.5" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-3)" />
      </Svg>
      </View>
  );
  }
}

class LinearGradientTypes extends Component {
  static title =
    'Horizontal vs Vertical linear gradient from yellow to red';
  render() {
    return (
     <View
        style={{
          width: 500,
          height: 200,
          flexDirection: 'row',
          justifyContent: 'space-around',
        }}>
        <Svg height="150" width="300" style={{backgroundColor:'black'}}>
          <Defs>
            <LinearGradient id='grad-horz' x1={0} y1={0} x2="100%" y2="0%">
              <Stop offset="0" stopColor="red" stopOpacity="0" />
              <Stop offset=".25" stopColor="orange" stopOpacity="0"/>
              <Stop offset=".5" stopColor="yellow" stopOpacity="0" />
              <Stop offset=".75" stopColor="green" stopOpacity="0"/>
              <Stop offset="1" stopColor="blue" stopOpacity="0"/>

             </LinearGradient>
          </Defs>
          <Rect x1="100" y1="50" x2="200" y2="150" fill="url(#grad-horz)" />
        </Svg>
        <Svg height="150" width="300">
         <Defs>
           <LinearGradient id="grad-vert" x1={0} y1={0} x2="0%" y2="100%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="1" />
             <Stop offset="100%" stopColor="red" stopOpacity="1" />
           </LinearGradient>
         </Defs>
         <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-vert)" />
        </Svg>
      </View>
    );
  }
}

class GradientUnits extends Component {
  static title = 'gradientUnits "OnObjectBoundingBox" vs "userSpaceOnUse" ';
  render() {
    return (
      <View
        style={{
          width: 220,
          height: 150,
          flexDirection: 'row',
          justifyContent: 'space-around',
        }}>
        <Svg height="150" width="90">
          <Defs>
            <LinearGradient id="defaultUnits" x1="0%" y1="0%" x2="0%" y2="100%">
              <Stop offset="0%" stopColor="#000" stopOpacity="1" />
              <Stop offset="100%" stopColor="#ff0" stopOpacity="1" />
            </LinearGradient>
          </Defs>
          <Rect
            fill="url(#defaultUnits)"
            x="10"
            y="10"
            width="70"
            height="70"
            rx="10"
            ry="10"
          />
        </Svg>
        <Svg height="150" width="90">
          <Defs>
            <LinearGradient
              id="userSpaceOnUse"
              x1="0%"
              y1="0%"
              x2="0%"
              y2="100%"
              gradientUnits="userSpaceOnUse">
              <Stop offset="0%" stopColor="#000" stopOpacity="1" />
              <Stop offset="100%" stopColor="#ff0" stopOpacity="1" />
            </LinearGradient>
          </Defs>
          <Rect
            fill="url(#userSpaceOnUse)"
            x="10"
            y="10"
            width="70"
            height="70"
            rx="10"
            ry="10"
          />
        </Svg>
      </View>
    );
  }
}

class LinearGradientWithOpacity extends Component {
  static title = 'linear gradient with stop Opacity  1 vs 0 vs .5';
  render() {
    return (
     <View
        style={{
          width: 800,
          height: 200,
          flexDirection: 'row',
          justifyContent: 'space-around',
	  backgroundColor :'cyan'
        }}>
      <Svg height="150" width="300">
        <Defs>
          <LinearGradient id="grad-3" x1="20%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="1" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-3)" />
      </Svg>

      <Svg height="150" width="300">
        <Defs>
          <LinearGradient id="grad-3" x1="20%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="0" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-3)" />
      </Svg>
      <Svg height="150" width="300">
        <Defs>
          <LinearGradient id="grad-3" x1="20%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="0.5" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Ellipse cx="150" cy="75" rx="85" ry="55" fill="url(#grad-3)" />
      </Svg>
      </View>
    );
  }
}

const icon = (
  <Svg height="30" width="30" viewBox="0 0 20 20">
    <Defs>
      <LinearGradient id="icon-grad" x1="0" y1="0" x2="0" y2="100%">
        <Stop offset="0" stopColor="blue" stopOpacity="1" />
        <Stop offset="100%" stopColor="red" stopOpacity="1" />
      </LinearGradient>
    </Defs>
      <LinearGradient id="icon-grad" x1="0" y1="0" x2="0" y2="100%">
        <Stop offset="0" stopColor="blue" stopOpacity="1" />
        <Stop offset="100%" stopColor="red" stopOpacity="1" />
      </LinearGradient>
    <Circle cx="10" cy="10" r="10" fill="url(#icon-grad)" />
  </Svg>
);

const samples = [
  LinearGradienttest,
  GradientUnits,
  LinearGradientWithOpacity
];

export {icon, samples};
