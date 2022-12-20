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

	this.ai_01 = [{a1: {animating : true},
		a2: {color: "blue"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "green"},
		a3: {size: 500},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "purple"},
		a3: {size: 200},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "#00FF0055"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "purple"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: true}}]

	this.ai_02 = [{a1: {animating : true},
		a2: {color: "green"},
		a3: {size: 500},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "yellow"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "red"},
		a3: {size: 200},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "#00FF0055"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: true}},

		{a1: {animating : true},
		a2: {color: "yellow"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}}]

	this.ai_03 = [{a1: {animating : true},
		a2: {color: "green"},
		a3: {size: 200},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "orange"},
		a3: {size: 100},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "blue"},
		a3: {size: 500},
		a4: {hidewhenStopped: false}},		

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: true}},

		{a1: {animating : true},
		a2: {color: "#00FF0055"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "white"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}}]

	this.ai_04 = [{a1: {animating : true},
		a2: {color: "gray"},
		a3: {size: 200},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "orange"},
		a3: {size: 100},
		a4: {hidewhenStopped: false}},

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "pink"},
		a3: {size: 500},
		a4: {hidewhenStopped: false}},		

		{a1: {animating : false},
		a2: {color: "green"},
		a3: {size: 300},
		a4: {hidewhenStopped: true}},

		{a1: {animating : true},
		a2: {color: "#FFFF0055"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}},

		{a1: {animating : true},
		a2: {color: "purple"},
		a3: {size: 300},
		a4: {hidewhenStopped: false}}]
    
		this.count = 0;
		this.index = 0;
	    	this.state = {
	      		size: this.size,
			animating: this.animating,
			hidesWhenStopped: this.hidesWhenStopped,
		      	qs: this.qs[this.count],
			ai_01: this.ai_01[this.count],
			ai_02: this.ai_02[this.count],
			ai_03: this.ai_03[this.count],
			ai_04: this.ai_04[this.count]}
		this.timer = null;    
  }
  
componentDidMount =() =>{
    	this.timer = setInterval(()=>{
      	console.log("-----------------timer fired--------:" + this.count)
      	this.count++;
      	if (this.count % 7 == 0){
		console.log("-----------ACTIVITY INDICATOR PROPERTIES UPDATED  INDEX = --------:" + this.index)
		this.index++
		this.setState ({
		qs: this.qs[this.count%4],
		ai_01: this.ai_01[this.index],
		ai_02: this.ai_02[this.index],
		ai_03: this.ai_03[this.index],
		ai_04: this.ai_04[this.index],
		count: this.count
		})

		if(this.index == 6){
			this.index = -1;
		}	 
      	} else {

		this.setState ({
		qs: this.qs[this.count%4],
		count: this.count
		}) 
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

        <View style={{margin: 50}}>
        <Text style={{fontSize: 20}}>{this.state.count}</Text>
        <View style={{ position: "absolute", top:30, left:0, width: 300, height: 300, backgroundColor: this.state.qs.q1.color}}></View>
        <View style={{ position: "absolute", top:30, left:300, width: 300, height: 300, backgroundColor: this.state.qs.q2.color}}></View>
        <View style={{ position: "absolute", top:330, left:0, width: 300, height: 300, backgroundColor: this.state.qs.q3.color}}></View>
        <View style={{ position: "absolute", top:330, left:300, width: 300, height: 300, backgroundColor: this.state.qs.q4.color}}></View>

	<View style={{ position: "absolute", top:30, left:610, width: 300, height: 300, backgroundColor: this.state.qs.q1.color}}></View>
        <View style={{ position: "absolute", top:30, left:910, width: 300, height: 300, backgroundColor: this.state.qs.q2.color}}></View>
        <View style={{ position: "absolute", top:330, left:610, width: 300, height: 300, backgroundColor: this.state.qs.q3.color}}></View>
        <View style={{ position: "absolute", top:330, left:910, width: 300, height: 300, backgroundColor: this.state.qs.q4.color}}></View>

	<View style={{ position: "absolute", top:30, left:1220, width: 300, height: 300, backgroundColor: this.state.qs.q1.color}}></View>
	<View style={{ position: "absolute", top:30, left:1520, width: 300, height: 300, backgroundColor: this.state.qs.q2.color}}></View>
	<View style={{ position: "absolute", top:330, left:1220, width: 300, height: 300, backgroundColor: this.state.qs.q3.color}}></View>
	<View style={{ position: "absolute", top:330, left:1520, width: 300, height: 300, backgroundColor: this.state.qs.q4.color}}></View>

          <ActivityIndicator animating={this.state.ai_01.a1.animating} color={this.state.ai_01.a2.color} size={this.state.ai_01.a3.size} hidesWhenStopped={this.state.ai_01.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 130, left: 130, 
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>

	<ActivityIndicator animating={this.state.ai_02.a1.animating} color={this.state.ai_02.a2.color} size={this.state.ai_02.a3.size} hidesWhenStopped={this.state.ai_02.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 130, left: 730, 
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>

	<ActivityIndicator animating={this.state.ai_03.a1.animating} color={this.state.ai_03.a2.color} size={this.state.ai_03.a3.size} hidesWhenStopped={this.state.ai_03.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 130, left: 1330, 
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>

	<ActivityIndicator animating={this.state.ai_02.a1.animating} color={this.state.ai_02.a2.color} size={this.state.ai_02.a3.size} hidesWhenStopped={this.state.ai_02.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 650, left: 130, 
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>

	<ActivityIndicator animating={this.state.ai_04.a1.animating} color={this.state.ai_04.a2.color} size={this.state.ai_04.a3.size} hidesWhenStopped={this.state.ai_04.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 550, left: 830, 
               borderWidth:20, borderColor:"green",shadowOpacity: 1,
               shadowColor: "red", shadowOffset:{width:30, height:30}}}/>

	<ActivityIndicator animating={this.state.ai_01.a1.animating} color={this.state.ai_01.a2.color} size={this.state.ai_01.a3.size} hidesWhenStopped={this.state.ai_01.a4.hidewhenStopped}  
              style={{position:"absolute",
               top: 650, left: 1330, 
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
