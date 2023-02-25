import React, { Component } from 'react';
import { AppRegistry, Dimensions, Text, View, StyleSheet, Image, TouchableOpacity, TouchableHighlight } from 'react-native';
import LinearGradient from 'react-native-linear-gradient';

const windowSize = Dimensions.get('window');

let gradientSquareWidth = Math.ceil(windowSize.width * 0.10);
let gradientSquareHeight = gradientSquareWidth;
let gradientSquareTextFontSize = gradientSquareWidth / 10;
let gradientRectWidth = Math.ceil(gradientSquareWidth * 1.75);
let gradientRectHeight = gradientSquareWidth;
let gradientLogoHeight = Math.ceil(windowSize.height * 0.07);
let gradientLogoWidth = gradientLogoHeight;
let gradientButtonHeight = gradientLogoHeight;
let gradientButtonWidth = gradientButtonHeight * 2;

console.log("************************************************ Window Size [" + windowSize.width + " x " + windowSize.height + "] " + gradientButtonWidth)
export default class SimpleViewApp extends Component {
  
  constructor(){
    super()
    this.state = {
      angle : 0
    }
    this.timer = null;
    this.count=0;
    this.MAX_COUNT = 1000;
  }

  componentWillUnmount = () => {
    clearInterval(this.timer)
    this.timer = null;
  }

  componentDidMount = () =>{
    this.timer = setInterval(() => {
        let a1 = this.state.angle + 10;
        if(a1 == 360){
          a1 = 0;
        }
        this.setState({angle: a1})
        this.count++;
        if(this.count > this.MAX_COUNT){
          clearInterval(this.timer)
          this.timer = null;
        }
      }, 100);
  }

  render(){
    return (
      <View style={styles.container}>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:0 , y: 1}}

          locations={[0.5, 0.5]}

          useAngle={true}
          angle = {this.state.angle}
          angleCenter={{x:0.5, y:0.5}}

          colors={['red','blue']}
          
          style={styles.linearGradient1}>
          <Image source={require("./1.png")} style={styles.logo}/> 
          <Text style={styles.buttonText}> CLockRotation + Image </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:0 , y: 1}}

          locations={[0.1, 0.9]}

          useAngle={true}
          angle = {this.state.angle}
          angleCenter={{x:0.5, y:0.5}}

          colors={['red','blue']}
          
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}> CLockRotation </Text>
        </LinearGradient>

        <LinearGradient
          colors={['blue','green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}> Default Pos/Loc </Text>
        </LinearGradient>

        <LinearGradient
          useAngle={true}
          colors={['blue','green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}> Default angle/center </Text>
        </LinearGradient>
        
        <LinearGradient
          useAngle={true}
          angle = {0}
          angleCenter={{x:0.5, y:0.5}}
          colors={['blue','green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}> 0 Deg center </Text>
        </LinearGradient>
        
        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 0}}
          useAngle={true}
          angle = {0}
          angleCenter={{x:0.5, y:0.5}}
          colors={['blue','green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}> Horizontal + 0 Deg </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:0 , y: 1}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Vertical solid 
          </Text>
        </LinearGradient>
        
        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:0 , y: 1}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          useAngle={true}
          angle = {0}
          angleCenter={{x:0.5, y:0.5}}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Vertical solid 0 deg
          </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 0}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Horizontal solid
          </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 0}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          useAngle={true}
          angle = {0}
          angleCenter={{x:0.5, y:0.5}}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Horizontal solid 0 deg
          </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 0}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          useAngle={true}
          angle = {45}
          angleCenter={{x:0.5, y:0.5}}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Horizontal solid 45 deg
          </Text>
        </LinearGradient>
        
        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 1}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          useAngle={true}
          angle = {0}
          angleCenter={{x:0.5, y:0.5}}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Diagonal solid 0 deg
          </Text>
        </LinearGradient>

        <LinearGradient
          start={{x: 0.0, y: 0.0}}
          end={{x:1 , y: 1}}
          locations={[0.25,0.25,0.25,0.5,0.5, 0.75, 0.75]}
          useAngle={true}
          angle = {45}
          angleCenter={{x:0.5, y:0.5}}
          colors={['red', '#00000000','yellow', 'yellow','blue', 'blue', 'green']}
          style={styles.linearGradient1}>
          <Text style={styles.buttonText}>
            Diagonal solid 45 deg
          </Text>
        </LinearGradient>

        <View style={{
          margin: 10,
          paddingLeft: 15,
          paddingRight: 15,
          borderWidth: 3,
          borderColor: "black",
          backgroundColor: "lightgrey",
          elevation: 50,
          width: gradientSquareWidth, 
          height: gradientSquareWidth,
          zIndex: 9,
          shadowOpacity: 1,
          shadowRadius: 30,
          shadowColor: "green",
          shadowOffset: {width: 10, height: 10}
        }}>
          <Text style={styles.buttonText}>View Without LG + Shadow </Text>
        </View>

        <LinearGradient
          colors={['red', 'yellow', 'green' ]}
          style={styles.linearGradient2}
          >
          <Text>Vertical Gradient</Text>
        </LinearGradient>

        <LinearGradient
          colors={['red', 'yellow', 'green' ]}
          style={styles.linearGradient2}
          start={{ x: 0.7, y: 0 }}
          >
          <Text>Diagonal Gradient</Text>
        </LinearGradient>

        <LinearGradient
          colors={['red', 'yellow', 'green' ]}
          style={styles.linearGradient2}
          start={{ x: 0, y: 0.5 }}
          end={{ x: 1, y: 0.5 }}
          >
          <Text>Horizontal Gradient</Text>
        </LinearGradient>

        <LinearGradient
          colors={['red', 'yellow', 'green' ]}
          style={styles.linearGradient2}
          start={{ x: 0, y: 0.5 }}
          end={{ x: 1, y: 0.5 }}
          locations={[0, 0.7, 0.9]}
          >
          <Text>H. Location Gradient</Text>
        </LinearGradient>

        <LinearGradient
          colors={['red', 'yellow', 'green' ]}
          style={styles.linearGradient2}
          locations={[0, 0.3, 0.9]}
          >
          <Text>V. Location Gradient</Text>
        </LinearGradient>

        <LinearGradient
          style={styles.container2}
          colors={['purple', 'white']}
          start={{ x: 0, y: 0 }}
          end={{ x: 1, y: 1 }}
          >
          <Text>Home Screen</Text>

          <TouchableOpacity onPress={() => {}}>
            <LinearGradient
              start={{ x: 0, y: 0 }}
              end={{x: 1, y: 1 }}
              colors={['#5851DB', '#C13584', '#E1306C', '#FD1D1D', '#F77737']}
              style={styles.instagramButton}
              >
              <Text style={{ color: 'white' }}>
                Sign In With Instagram
              </Text>
            </LinearGradient>
          </TouchableOpacity>

          <LinearGradient
            start={{ x: 0, y: 0 }}
            end={{x: 1, y: 1 }}
            colors={[ 'red', 'yellow', 'green' ]}
            style={{
              alignItems: 'center',
              justifyContent: 'center',
              borderRadius: 10,
            }}
            >
            <TouchableOpacity
              onPress={() => {}}
              style={styles.signUpButton}
              >
              <Text>Sign Up</Text>
            </TouchableOpacity>
          </LinearGradient>
        </LinearGradient>
    
      </View>

    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 2,
    flexWrap: "wrap",
    flexDirection: "row",
    justifyContent: 'center',
    position: "absolute",
    margin: 20
  },
  container2: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
    margin: 10
  },
  logo:{
    width: gradientLogoWidth,
    height: gradientLogoHeight,
    marginTop: 10,
    borderColor: "red",
    borderWidth: 1,
    resizeMode: "stretch",
    opacity: 0.7
  },
  linearGradient1: {
    margin: 10,
    paddingLeft: 10,
    paddingRight: 10,
    borderWidth: 3,
    borderColor: "brown",
    shadowColor: "red",
    height: gradientSquareWidth,
    width: gradientSquareWidth,
    justifyContent: "center",
    alignItems: "center",
    shadowOpacity: 1,
    shadowRadius: 3,
    shadowColor: "grey",
    shadowOffset: {width: 5, height: 5}
  },
  linearGradient2: {
    margin: 10,
    paddingLeft: 10,
    paddingRight: 10,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 5,
    width: gradientRectWidth,
    height: gradientRectHeight,
    shadowOpacity: 1,
    shadowRadius: 3,
    shadowColor: "grey",
    shadowOffset: {width: 5, height: 5}
  },
  buttonText: {
    fontSize: gradientSquareTextFontSize,
    textAlign: 'center',
    margin: 10,
    color: 'red',
    
    width: gradientButtonWidth,
    height: gradientButtonHeight,
    borderColor: "black",
    borderWidth: 1,
    backgroundColor: "#ffffff55"
  },
  instagramButton: {
    fontSize: gradientSquareTextFontSize,
    paddingHorizontal: 40,
    paddingVertical: 10,
    borderRadius: 10,
    margin: 20
  },
  signUpButton: {
    fontSize: gradientSquareTextFontSize,
    margin: 1,
    width: gradientSquareWidth,
    borderRadius: 10,
    paddingVertical: 10,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: 'white',
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
