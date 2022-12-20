import React, { Component } from "react";
import { AppRegistry, ActivityIndicator, StyleSheet, Text, View } from "react-native";

class SimpleViewApp extends Component {
  constructor(props){
    super(props)
    this.qs = [{q1: {color: "yellow"},
               q2: {color: "lightblue"},
               q3: {color: "pink"},
               q4: {color: "lightsalmon"}}, 
               
               {q2: {color: "yellow"},
               q3: {color: "lightblue"},
               q4: {color: "pink"},
               q1: {color: "lightsalmon"}},

               {q3: {color: "yellow"},
               q4: {color: "lightblue"},
               q1: {color: "pink"},
               q2: {color: "lightsalmon"}}, 
               
               {q4: {color: "yellow"},
               q1: {color: "lightblue"},
               q2: {color: "pink"},
               q3: {color: "lightsalmon"}}]
    
    this.count = 0;
    this.size = 300;
    this.state = {
      size: this.size,
      qs: this.qs[this.count],
      box: {w: 100, h: 100}
    }
    this.timer = null;    
  }

  randomUpdate = () => {
     this.timer = setInterval(()=>{
      let box = {w:100, h:100}
      console.log("-----------------timer fired--------:" + this.count)
      this.count++;
      if (this.count % 5 == 0){
        //clearInterval(this.timer)
        this.size = 300;
      } else if (this.count % 7 == 0){
        this.size = 100;
        box = {w:80, h: 80}
      } else if (this.count % 11 == 0) {
        this.size = 50;
        box = {w:180, h: 180}
      }
      this.setState ( 
        {
          size: this.size,
          qs: this.qs[this.count%4],
          count: this.count,
          box: box
        }
      )

    }, 1000)
  }

  spinnerSizeUpdate = () => {
    this.timer = setInterval(()=>{
      let box = {w:100, h:100}
      let size = 400
      console.log("---------------timer fired--------:" + this.count)
      this.count++;
      if (this.count / 5 < 1){
        box = {w: this.state.box.w+50, h: this.state.box.h+50}
      } else if (this.count / 5 < 2){
        box = {w: this.state.box.w-50, h: this.state.box.h-50}
      } else{
        size = this.state.size - 50;
      }
      this.setState ( 
        {
          size: size,
          count: this.count,
          box: box
        }
      )
      if(this.count > 15){
        clearInterval(this.timer)
        this.timer = null;
        this.randomUpdate()
      }

    }, 1000)

  }
  
  componentDidMount =() =>{
      this.spinnerSizeUpdate();
  }

  componentWillUnmount = () => {
    console.log("-------------------unmount ----------------")
    clearInterval(this.timer)
  }
   
  render() {
    return (
      <View >

        <View style={{margin: 50}}>
          <Text style={{fontSize: 20}}>{this.state.count}</Text>
          <View style={{ position: "absolute", top:30, left:0, width: 300, height: 300, backgroundColor: this.state.qs.q1.color}}></View>
          <View style={{ position: "absolute", top:30, left:300, width: 300, height: 300, backgroundColor: this.state.qs.q2.color}}></View>
          <View style={{ position: "absolute", top:330, left:0, width: 300, height: 300, backgroundColor: this.state.qs.q3.color}}></View>
          <View style={{ position: "absolute", top:330, left:300, width: 300, height: 300, backgroundColor: this.state.qs.q4.color}}></View>

          
          <ActivityIndicator size={this.state.size} color="#0000ff" hidesWhenStopped={false} animating={true} 
              style={{position:"absolute",
               top: 280, left: 250,
               width: this.state.box.w, height: this.state.box.h,
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>
        </View>
      </View>
    );
  }
}

const styles = StyleSheet.create({

});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
