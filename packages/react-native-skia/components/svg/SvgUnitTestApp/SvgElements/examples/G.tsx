import React, {Component} from 'react';
import {Svg, Circle, G, Text, Line, Rect, Use,Defs} from 'react-native-svg';

class GExample1 extends Component {
  static title = 'Group of circles';
  render(){
    return (
      <Svg  height="150" width="150" >
        <G fill="white" stroke="green" strokeWidth="5">
          <Circle cx="40" cy="40" r="25" />
          <Circle cx="60" cy="60" r="25" />
        </G>
      </Svg>
    );
  }  
}

class UseExample extends Component {
  static title = 'Group with Reuse Svg code';
  render() {
    return (
      <Svg height="100" width="300">
        <Defs>
          <G id="reuse-shape">
            <Circle cx="50" cy="50" r="50" />
            <Rect x="50" y="50" width="50" height="50" />
            <Circle cx="50" cy="50" r="5" fill="blue" />
          </G>
        </Defs>
        <Use href="#reuse-shape" x="20" y="0" />
        <Use href="#reuse-shape" x="170" y="0" />
      </Svg>
    );
  }
}

class GExample extends Component {
  static title = 'G children props inherit';
  state = {
    fill: 'blue',
  };
  componentDidMount = () => {
    setTimeout(() => {
      if (!this._unmounted) {
        this.setState({
          fill: 'purple',
        });
      }
    }, 3000);
  };
  componentWillUnmount = () => {
    this._unmounted = true;
  };
  private _unmounted: any;
  render() {
    return (
      <Svg height="100" width="100">
        <G fill={this.state.fill} stroke="pink" strokeWidth="3" opacity='.5'>
          <Circle cx="25" cy="25" r="11" />
          <Circle cx="25" cy="75" r="11" stroke="red" />
        </G>
        <G fill="yellow" >
          <Circle cx="50" cy="50" r="11" fill="green" />
          <Circle cx="75" cy="25" r="11" stroke="red" />
          <Circle cx="75" cy="75" r="11" />
        </G>
      </Svg>
    );
  }
}

class GTransform extends Component {
  static title = 'G transform';
  render() {
    return (
      <Svg height="100" width="200">
        <G rotation="50" origin="40, 30" id="group">
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
          x="5"
          y="40"
          rotation="-50"
          scale="0.75"
          stroke="red"
          opacity="0.5"
        />
      </Svg>
    );
  }
}

const samples = [GExample1,UseExample,GExample,GTransform];

export { samples};
