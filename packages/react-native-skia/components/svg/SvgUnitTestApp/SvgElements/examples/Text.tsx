import React, {Component} from 'react';

import {
  Svg,
  G,
  Text,
  TSpan,
  TextPath,
  Path,
  Defs,
  LinearGradient,
  Stop,
} from 'react-native-svg';

class TextExample extends Component {
  static title = 'Text';
  render() {
    return (
      <Svg height="50" width="100">
        <Text x="50" y="15" fill="red" textAnchor="middle">
          I love SVG!
        </Text>
      </Svg>
    );
  }
}

class TextStroke extends Component {
  static title = 'Stroke the text';
  render() {
    return (
      <Svg height="80" width="200">
        <Defs>
          <LinearGradient
            id="text-stroke-grad"
            x1="0%"
            y1="0%"
            x2="100%"
            y2="0%">
            <Stop offset="0%" stopColor="green" stopOpacity="0.5" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Text
          stroke="url(#text-stroke-grad)"
          strokeWidth="3"
          fill="none"
          fontSize="20"
          fontWeight="bold"
          x="100"
          y="30">
          <TSpan textAnchor="middle">STROKE TEXT</TSpan>
        </Text>
      </Svg>
    );
  }
}

class TextFill extends Component {
  static title = 'Fill the text with LinearGradient';
  render() {
    return (
      <Svg height="180" width="300">
        <Defs>
          <LinearGradient id="text-fill-grad" x1="0%" y1="0%" x2="100%" y2="0%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="0.5" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>

        <Text
          fill="url(#text-fill-grad)"
          stroke="purple"
          strokeWidth="1"
          fontSize="34"
          fontWeight="bold"
          x="100"
          y="30"
          textAnchor="middle">
          FILL TEXT
        </Text>
      </Svg>
    );
  }
}

class TSpanExample extends Component {
  static title = 'TSpan nest';
  render() {
    return (
      <Svg height="160" width="200">
        <Text y="20" dx="5 5">
          <TSpan x="10">tspan line 1</TSpan>
          <TSpan x="10" dy="15">
            tspan line 2
          </TSpan>
          <TSpan x="10" dx="10" dy="15">
            tspan line 3
          </TSpan>
        </Text>
        <Text x="10" y="60" fill="red" fontSize="14">
          <TSpan dy="5 10 20">12345</TSpan>
          <TSpan fill="blue" dy="15" dx="0 5 5">
            <TSpan>6</TSpan>
            <TSpan>7</TSpan>
          </TSpan>
          <TSpan dx="0 10 20" dy="0 20" fontWeight="bold" fontSize="12">
            89a
          </TSpan>
        </Text>
        <Text y="140" dx="0 5 5" dy="0 -5 -5">
          delta on text
        </Text>
      </Svg>
    );
  }
}

const samples = [
  TextExample,
  TextStroke,
  TextFill,
  TSpanExample,
];

export {samples};
