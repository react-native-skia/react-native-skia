import React, {Component} from 'react';
import {Svg, Circle, G,Text, Line, Rect, Ellipse,Use,Path,LinearGradient,TSpan,Stop,Defs,Polygon,Polyline} from 'react-native-svg';
import { AppRegistry, StyleSheet, View } from "react-native";


class CircleExample  extends Component {
  static title = 'Circle';
  render() {
    return (
      <Svg height="700" width="700" viewBox="10 10 700 700" transform={[{translate:-10},{translateY:-10}]}>
        <Circle cx="150" cy="150" r="10%" fill="rgb(0,0,255)"
          id="circle"
          strokeOpacity=".7"
          fillOpacity=".3"
          strokeDasharray="5,10"
          strokeLinecap="butt"
          transform="translate(220)"
        />
      </Svg>
    );
  }
}

class RectExample extends Component {
  static title = 'Rect';
  render() {
    return (
      <View>
      <Svg x="200" y="100" width="200" height="200" >
        <G style={styles.container}>
        <Rect
          x="5"
          y="5"
          width="50"
          height="50"
          fill="rgb(0,0,255)"
        />
        </G>
      </Svg>
      </View>
    );
  }
}

class EllipseExample extends Component {
  static title = 'Ellipse';
  render() {
    return (
      <Svg width="200" height="200" fill="red"  strokeWidth={20}>
      <Ellipse cx={29.5} cy={39.896} stroke="#35FFA7" fill="none" rx={28.938} ry={39.333} />
      <Ellipse cx="240" cy="100" rx="220" ry="30" style="fill:purple" />
      <Ellipse cx="220" cy="70" rx="190" ry="20" style="fill:lime" />
      <Ellipse cx="210" cy="45" rx="170" ry="15" style="fill:yellow" />
      </Svg>
    );
  }
}
class GExample extends Component {
  static title = 'G children props inherit';
  state = {
    fill: 'purple',
  };
  componentDidMount = () => {
    setTimeout(() => {
        this.setState({
          fill: '#856',
        });
          }, 2000);
  };

  render() {
    return (
      <Svg height="100" width="100">
        <G  stroke="pink" strokeWidth="3">
          <Circle cx="25" cy="75" r="11" stroke="red" fill={this.state.fill}/>
          <Circle cx="75" cy="75" r="11" stroke="red" />
        </G>
        <G fill={this.state.fill} stroke="pink" strokeWidth="3">
            <Circle cx="25" cy="115" r="11" stroke="green" />
          </G>
      </Svg>
    );
  }
}


class PathExample extends Component {
  static title = 'Unclosed paths';
  render() {
    return (
      <Svg height="200" width="200">
        <Path
          //d="M25 10 L98 65 L70 25 L16 77 L11 30 L0 4 L90 50 L50 10 L11 22 L77 95 L20 25"
          //d="M 243 128 L 24 178 L 200 38 L 102 240 L 102 16 L 200 218 L 24 78 Z" // star
          d="M20.563 2.44h-6.145c-.048-.508-.216-1.365-.793-1.916C13.26.177 12.797 0 12.25 0H7.875C7.327 0 6.865.177 6.5.524c-.577.55-.746 1.408-.794 1.917H.437c-.241 0-.437.182-.437.406 0 .225.196.407.438.407h.892l.838 18.72c.016.701.485 2.026 2.147 2.026h12.37c1.662 0 2.13-1.325 2.146-2.017l.839-18.729h.892c.242 0 .438-.182.438-.407 0-.224-.196-.406-.438-.406zm-14 17.899c0 .225-.196.407-.438.407s-.438-.182-.438-.407V6.915c0-.225.196-.407.438-.407s.438.182.438.407V20.34zm4.375 0c0 .225-.197.407-.438.407-.242 0-.438-.182-.438-.407V6.915c0-.225.197-.407.438-.407.242 0 .438.182.438.407V20.34zm4.374 0c0 .225-.195.407-.437.407s-.438-.182-.438-.407V6.915c0-.225.197-.407.438-.407.242 0 .438.182.438.407V20.34zM7.129 1.091c.196-.187.44-.277.747-.277h4.375c.306 0 .55.09.747.277.352.334.493.923.542 1.35H6.586c.049-.427.19-1.016.542-1.35z"
          fill="none"
          stroke="red"
          strokeWidth="1"
        />
      </Svg>
    );
  }
}

class PolygonExample extends Component {
  static title = 'The following example creates a polygon with three sides';
  render() {
    return (
      <Svg height="300" width="200">
        <Polygon
         // points="40,5 70,80 25,95"
          //points="70 5 90  75 45 90 25 80"
           points="100,10 40,198 190,78 10,78 160,198"
         // fill="lime"
         // stroke="purple"
          fillRule="evenodd"
          strokeWidth="5"
          style={{fill: "skyblue",stroke: "cadetblue"}}
        />
      </Svg>
    );
  }
}

class PolylineExample extends Component {
  static title =
    'The <Polyline> element is used to create any shape that consists of only straight lines';
  render() {
    return (
      <Svg height="100" width="100" >
        <Polyline
          //points="10 10 20 12 30 20 40 60 60 70 95 90"
          points="0,20 20,20 20,40 40,40 40,60 60,60 60,80"
          stroke="black"
          strokeWidth="3"
        />
      </Svg>
    );
  }
}

class LineExample extends Component {
  static title = 'Line';
  render() {
    return (
      <Svg height="100" width="100">
        <Line
          x1="10%"
          y1="10%"
          x2="90%"
          y2="90%"
          stroke="red"
          strokeWidth="2"
        />
      </Svg>
    );
  }
}

class UseExample extends Component {
  static title = 'Reuse svg code';
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

class Test extends Component {
  static title = 'GRoupExample';
  state = {
    fill: 'purple',
  };
  componentDidMount = () => {
    setTimeout(() => {
        this.setState({
          fill: '#856',
        });
          }, 2000);
  };

  render() {
    return (
      <Svg height="700" width="700" viewBox="0 0 700 700" color="red" opacity='1'>
      <G opacity='1'  fillOpacity=".6" stroke="purple" fill='none' strokeWidth={10} strokeOpacity=".4"
          transform="translate(50)"id="Group">
      <Path
          //d="M25 10 L98 65 L70 25 L16 77 L11 30 L0 4 L90 50 L50 10 L11 22 L77 95 L20 25"
          //d="M 243 128 L 24 178 L 200 38 L 102 240 L 102 16 L 200 218 L 24 78 Z" // star
          d="M20.563 2.44h-6.145c-.048-.508-.216-1.365-.793-1.916C13.26.177 12.797 0 12.25 0H7.875C7.327 0 6.865.177 6.5.524c-.577.55-.746 1.408-.794 1.917H.437c-.241 0-.437.182-.437.406 0 .225.196.407.438.407h.892l.838 18.72c.016.701.485 2.026 2.147 2.026h12.37c1.662 0 2.13-1.325 2.146-2.017l.839-18.729h.892c.242 0 .438-.182.438-.407 0-.224-.196-.406-.438-.406zm-14 17.899c0 .225-.196.407-.438.407s-.438-.182-.438-.407V6.915c0-.225.196-.407.438-.407s.438.182.438.407V20.34zm4.375 0c0 .225-.197.407-.438.407-.242 0-.438-.182-.438-.407V6.915c0-.225.197-.407.438-.407.242 0 .438.182.438.407V20.34zm4.374 0c0 .225-.195.407-.437.407s-.438-.182-.438-.407V6.915c0-.225.197-.407.438-.407.242 0 .438.182.438.407V20.34zM7.129 1.091c.196-.187.44-.277.747-.277h4.375c.306 0 .55.09.747.277.352.334.493.923.542 1.35H6.586c.049-.427.19-1.016.542-1.35z"
          stroke="green"
          strokeWidth="1"
          transform={[{scale:2}]}
      />
       <Ellipse
          id="Ellipse"
          cx={39.5}
          cy={99.896}
          stroke="#35FFA7"
          rx={28.938}
          ry={39.333}
          fill="red"
      />
      
       <Rect
          id="Rect"
          x="100px"
          y="200px"
          width="20%"
          height="180"
          fill="rgb(0,0,255)"
          strokeWidth="3"
          strokeOpacity=".7"
          strokeWidth="2"
          stroke="rgb(0,0,0)"
          strokeDasharray="5,10"
          strokeLinecap="butt"
          transform="translate(220)"
        />
      <Circle cx="150" cy="150" r="10%" fill="rgb(0,0,255)"
          strokeWidth="3"
          id="circle"
          strokeOpacity=".7"
          fillOpacity=".8"
          strokeDasharray="5,10"
          strokeLinecap="butt"
          transform="translate(220)"
          />
          </G>
      </Svg>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    marginTop: 100,
    alignContent:'center',
    justifyContent:'center',
    backgroundColor: "lightgrey",
    width: 500,
    height: 500,
    borderColor:'red',
    borderWidth:5,
    left:100
  }
});


class viewBoxExample extends Component {
  static title =
    'The <Polyline> element is used to create any shape that consists of only straight lines';
  render() {
    return (
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
    );
  }
}

AppRegistry.registerComponent('SimpleViewApp', () => Test );