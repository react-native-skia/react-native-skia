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

class FillNone extends Component {
  static title = 'Fill:none';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          fill="none"
          />
        </Svg>
    );
  }
}

class FillCurrentColor extends Component {
  static title = 'Fill:currentColor';
  render() {
    return (
      <Svg height="100" width="100" color='yellow'>
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          fill="currentColor"
          />
        </Svg>
    );
  }
}

class FillWithLinearGradient extends Component {
  static title = 'Fill:LinearGradient';
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
          fill="url(#grad-3)"
        />
        </Svg>
    );
  }
}

class FillDefault extends Component {
  static title = 'Fill:Default';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          />
        </Svg>
    );
  }
}

class FillOpacity extends Component {
  static title = 'FillOpacity:0.5';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          fill="green"
          fillOpacity='0.5'
          />
        </Svg>
    );
  }
}

class FillSolidColor extends Component {
  static title = 'Fill:solidColor';
  render() {
    return (
      <Svg height="100" width="100">
        <Circle
          cx="50"
          cy="50"
          r="45"
          stroke="blue"
          strokeWidth="2.5"
          fill="green"
          />
        </Svg>
    );
  }
}

class FillRule extends Component {
  static title = 'FillRule:"evenOdd" vs "nonzero"';
  render() {
    return (
    <View style={styles.boxView}>
      <Svg height="105" width="105">
        <G scale="0.5" fillRule="evenodd">
          <Polygon
            points="100,10 40,198 190,78 10,78 160,198"
            fill="lime"
            stroke="purple"
            strokeWidth="5"
          />
        </G>
      </Svg>
      <Svg height="105" width="105">
        <G scale="0.5">
          <Polygon
            points="100,10 40,198 190,78 10,78 160,198"
            fill="lime"
            stroke="purple"
            strokeWidth="5"
          />
        </G>
      </Svg>
    </View>
    );
  }
}


const samples = [
  FillSolidColor,
  FillOpacity,
  FillDefault,
  FillNone,
  FillCurrentColor,
  FillWithLinearGradient,
  FillRule
];

export { samples};
