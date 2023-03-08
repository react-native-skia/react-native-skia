import React, { Component } from "react";
import { AppRegistry, ActivityIndicator, StyleSheet, Text, View } from "react-native";

class SimpleViewApp extends Component {
  
  constructor(props){
    super(props)
    this.state = {
      aiList : null,
      count: 0
    }
    this.timer = null;
    this.count = 0;
  }



  componentDidMount = () => {
    let aiList3 = (
      <>
      <ActivityIndicator size={30} color="red" />
      <ActivityIndicator size={40} color="blue"/>
      <ActivityIndicator size={50} color="green"/>
      </>
    );
    let aiList1 = (
      <>
      
      </>
    );
    this.timer = setInterval(()=>{
      this.count++;
      console.log("----- timer fired count:"+ this.count)

	
	if(this.count % 5 == 0){
	      if(this.count % 10 == 0){
		this.setState({ aiList : aiList3, count: this.count })
	      } else {
		this.setState({ aiList: aiList1, count: this.count })        
	      }
	}
    }, 1000)
  }

  componentWillUnmount = () => {
    clearInterval(this.timer)
    this.timer = null;
  }

  render() {
    return (
      <View style={styles.container}>
        <Text style={[styles.text,styles.button]}>{this.state.count}</Text>
        <View style={{position: "absolute", top:50, left:50}}>
        {this.state.aiList}
        </View>
      </View>
    );
  }
}

const styles = StyleSheet.create(
  {
    container:{
      borderWidth: 1,
      borderColor: "red",
      height:720,
      width:1280,
      display : "flex",
      flexWrap: "wrap"
    },
    button: {
      backgroundColor: "lightgrey",
      borderWidth: 1,
      width: 600,
      height: 25      
    },
    text: {
      fontWeight: "bold",
      textAlign: "center",
      textAlignVertical: "center"
    }

  }
)

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
