import React, {Component} from 'react';
import {StyleSheet, View, Image} from 'react-native';
import {Svg, Circle, G, Path, Line, Rect,LinearGradient,Use,Defs,Stop,Polygon} from 'react-native-svg';

const styles = StyleSheet.create({
  container: {
    flex: 1,
    height: 100,
    width: 200,
  },
  svg: {
    flex: 1,
    alignSelf: 'stretch',

  },
  boxView:{
    flexDirection:'row',
    paddingHorizontal:50
  }
});

class StrokeNone extends Component {
  static title = 'Stroke:none';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="none"
          strokeWidth="2.5"
          fill="none"
        />
        </Svg>
    );
  }
}

class StrokeCurrentColor extends Component {
  static title = 'Stroke:currentColor';
  render() {
    return (
      <Svg height="100" width="100" color='yellow'>
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          stroke="currentColor"
          fill="none"
          />
        </Svg>
    );
  }
}

class StrokeWithLinearGradient extends Component {
  static title = 'Stroke:LinearGradient';
  render() {
    return (
      <Svg height="100" width="100">
        <Defs>
          <LinearGradient id="grad-3" x1="0%" y1="0%" x2="0%" y2="100%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="1" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>

        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          stroke="url(#grad-3)"
          fill="none"
          />

        </Svg>
    );
  }
}

class StrokeDefault extends Component {
  static title = 'Stroke:Default';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          strokeWidth="2.5"
          fill="none"
          />

        </Svg>
    );
  }
}

class StrokeOpacity extends Component {
  static title = 'StrokeOpacity:0.5';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          stroke="green"
          strokeOpacity='0.5'
          fill="none"
          />

        </Svg>
    );
  }
}

class StrokeSolidColor extends Component {
  static title = 'Stroke:solidColor';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          stroke="green"
          fill="none"
          />

        </Svg>
    );
  }
}

const samples = [
  StrokeSolidColor,
  StrokeOpacity,
  StrokeDefault,
  StrokeNone,
  StrokeCurrentColor,
  StrokeWithLinearGradient,
];

export { samples};
