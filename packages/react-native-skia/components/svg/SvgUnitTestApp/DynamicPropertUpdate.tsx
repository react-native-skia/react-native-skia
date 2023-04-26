import React, {Component} from 'react';
import { View, AppRegistry,Text } from 'react-native';
import {Svg, Circle, G, Line, Rect, Use,Ellipse,Path,Stop,Defs,LinearGradient,Polygon,PolyLine} from 'react-native-svg';


export default class SvgDynamicPropertyUpdate extends Component {
  
  getExample =() => {
    return samples.map(Element => {
      return (
        <View >
          <Text >{Element.title}</Text>
          <Element />
        </View>    );} )
  };

  render() {
    return (
      <View
        style={{
          flexDirection: 'row',
          justifyContent: 'space-around',
        }}>
        {this.getExample()}
      </View>
    )
  }
}

class GExample extends Component {
  static title = 'Dynamic Update for Props : fill';
  state = {
    fill1: 'blue',
    fill2: 'blue',
  };
  componentDidMount = () => {
    setTimeout(() => {
      if (!this._unmounted) {
        this.setState({
          fill1: 'purple',
          fill2: 'yellow',
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
      <Svg height="150" width="100">
        <G fill={this.state.fill1} stroke="pink" strokeWidth="3" opacity='.5'>
          <Circle cx="25" cy="25" r="11" />
          <Circle cx="25" cy="75" r="11" stroke="red" />
        </G>
        <G fill={this.state.fill2} >
          <Circle cx="50" cy="50" r="11"  />
          <Circle cx="75" cy="25" r="11" stroke="red" />
          <Circle cx="75" cy="75" r="11" />
        </G>
      </Svg>
    );
  }
}

class CirclePropsUpdate extends Component {
  static title = 'Dynamic Update for Props : strokeWidth';
  state = {
    fill: 'yellow',
    strokeWidth :'0',
  };
  componentDidMount = () => {
    setTimeout(() => {
      if (!this._unmounted) {
        this.setState({
          fill: 'mediumpurple',
	  strokeWidth :'10',
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
      <Svg height="150" width="140">
        <Circle cx="50%" cy="50%" r="40%" fill="pink" stroke='red' strokeWidth={this.state.strokeWidth}
/>
      </Svg>

    );
  }
}

class RectPropsUpdate extends Component {
  static title = 'Dynamic Update for Props : fillopacity';
  state = {
    fillOpacity: '1',
  };
  componentDidMount = () => {
    setTimeout(() => {
      if (!this._unmounted) {
        this.setState({
          fillOpacity: '0.5',
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
      <Svg width="200" height="60">
        <Rect
          x="5%"
          y="5%"
          width="90%"
          height="90%"
          fill='purple'
          fillOpacity={this.state.fillOpacity}
          strokeWidth="3"
          stroke='black'
        />
      </Svg>
    );
  }
}
const samples = [
  GExample, 
  CirclePropsUpdate,
  RectPropsUpdate
];

































