import React, {Component} from 'react';
import {
  Svg,
  Circle,
  G,
  Text,
  Path,
  Polyline,
  Rect,
  Defs,
  RadialGradient,
  Stop,
  ClipPath,
} from 'react-native-svg';

class StrokeExample extends Component {
  static title =
    'The stroke property defines the color of a line, text or outline of an element';
  render() {
    return (
      <Svg height="80" width="225">
        <G strokeWidth="1">
          <Path stroke="red" d="M5 20 l215 0" />
          <Path stroke="blue" d="M5 40 l215 0" />
          <Path stroke="green" d="M5 60 l215 0" />
        </G>
      </Svg>
    );
  }
}

class StrokeLinecap extends Component {
  static title =
    'The strokeLinecap property defines different types of endings to an open path';
  render() {
    return (
      <Svg height="80" width="225">
        <G stroke="red" strokeWidth="8">
          <Path strokeLinecap="butt" d="M5 20 l215 0" />
          <Path strokeLinecap="round" d="M5 40 l215 0" />
          <Path strokeLinecap="square" d="M5 60 l215 0" />
        </G>
      </Svg>
    );
  }
}

class StrokeDasharray extends Component {
  static title = 'strokeDasharray';
  render() {
    return (
      <Svg height="80" width="225">
        <G fill="none" stroke="blue" strokeWidth="4">
          <Path strokeDasharray="5,5" d="M5 20 l215 0" />
          <Path strokeDasharray="10,10" d="M5 40 l215 0" />
          <Path strokeDasharray="20,10,5,5,5,10" d="M5 60 l215 0" />
        </G>
      </Svg>
    );
  }
}

class StrokeDashoffset extends Component {
  static title =
    'the strokeDashoffset attribute specifies the distance into the dash pattern to start the dash.';
  render() {
    return (
      <Svg height="80" width="200">
        <Circle
          cx="100"
          cy="40"
          r="35"
          strokeWidth="5"
          stroke="red"
          fill="none"
          strokeDasharray="100"
          strokeDashoffset="100"
        />
        <Text
          stroke="blue"
          strokeWidth="1"
          fill="none"
          fontSize="20"
          fontWeight="bold"
          fontFamily='MonoSpace'
          x="100"
          y="40"
          textAnchor="middle"
          strokeDasharray="100"
          strokeDashoffset="60">
          STROKE
        </Text>
      </Svg>
    );
  }
}

const samples = [
  StrokeExample,
  StrokeLinecap,
  StrokeDasharray,
  StrokeDashoffset,
];
export {samples};
