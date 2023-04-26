import React, {Component} from 'react';
import {Svg, Circle, G, Text, Line, Rect, Use} from 'react-native-svg';

class orginal extends Component {
  static title = 'orginal';
  render() {
    return (
      <Svg height="150" width="200" style={{borderColor:'blue', borderWidth:2}}>
        <G >
          <Rect
            height="50"
            width="80"
            stroke="#060"
            strokeWidth="1"
            fill="#060"
          />
        </G>
      </Svg>
    );
  }
}

class Scale extends Component {
  static title = 'Scale by 2x';
  render() {
    return (
      <Svg height="150" width="200" style={{borderColor:'blue', borderWidth:2}}>
        <G scale="2" >
          <Rect
            height="50"
            width="80"
            stroke="#060"
            strokeWidth="1"
            fill="#060"
          />
        </G>
      </Svg>
    );
  }
}

class Translate extends Component {
  static title = 'Translate by 50,50';
  render() {
    return (
      <Svg height="150" width="200" style={{borderColor:'blue', borderWidth:2}}>
        <G translate="50"  >
          <Rect
            height="50"
            width="80"
            stroke="#060"
            strokeWidth="1"
            fill="#060"
          />
        </G>
      </Svg>
    );
  }
}

class Rotate extends Component {
  static title = 'Rotate  by 20 deg';
  render() {
    return (
      <Svg height="150" width="200" style={{borderColor:'blue', borderWidth:2}}>
        <G rotation="20">
          <Rect
            height="50"
            width="80"
            stroke="#060"
            strokeWidth="1"
            fill="#060"
          />
        </G>
      </Svg>
    );
  }
}

class GTransform extends Component {
  static title = 'Group Rotate by -50 deg';
  render() {
    return (
      <Svg height="200" width="300" style={{borderColor:'blue', borderWidth:2}}>
        <G rotation="50" id="group">
          <Line
            x1="60"
            y1="10"
            x2="140"
            y2="10"
            stroke="#060"
            strokeWidth="1"
          />
          <Rect
            x="60"
            y="20"
            height="50"
            width="80"
            stroke="#060"
            strokeWidth="1"
            fill="#060"
          />
          <Text
            x="100"
            y="75"
            stroke="#600"
            strokeWidth="1"
            fill="#600"
            textAnchor="middle">
            Text grouped with shapes
          </Text>
        </G>
        <Use
          href="#group"
          rotation="-50"
          scale="0.75"
          stroke="red"
          opacity="0.5"
        />
      </Svg>
    );
  }
}

const samples = [orginal,Scale,Translate,Rotate,GTransform];

export { samples};
