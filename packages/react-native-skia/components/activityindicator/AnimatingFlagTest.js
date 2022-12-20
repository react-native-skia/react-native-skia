import React, { Component } from "react";
import { AppRegistry, ActivityIndicator, StyleSheet, Text, View } from "react-native";

class SimpleViewApp extends Component {
  constructor(props){
    super(props)
    
    this.count = 0;
    this.animating = true;
    this.state = {
      animating: this.animating
    }
    this.timer = null;    
  }
  
  componentDidMount =() =>{
    this.timer = setInterval(()=>{
      console.log("-----------------timer fired--------:" + this.count)
      this.count++;
	if (this.count % 5 == 0)
	{
        	if (this.count % 10 == 0){
		this.animating = true;
		}else{
		this.animating = false;
		}
	this.setState ( 
        {
          animating: this.animating,
          count: this.count
        }
      )

      }

    }, 1000)

  }

  componentWillUnmount = () => {
    console.log("-------------------unmount ----------------")
    clearInterval(this.timer)
  }
   
  render() {
    return (
      <View >
          <View style={{ position: "absolute", top:30, left:0, width: 1000, height: 1000, backgroundColor: "yellow"}}></View>
          <ActivityIndicator size={300} color="#0000ff" hidesWhenStopped={false} animating={this.state.animating} 
              style={{position:"absolute",
               top: 200, left: 200}}/>
      </View>
    );
  }
}

const styles = StyleSheet.create({

});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
