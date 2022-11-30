import * as React from 'react';
import { AppRegistry, View, ActivityIndicator } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View style={{ backgroundColor: "#FFBAABFF" ,left:50, top:50, width:800, height:800 }}>
      <ActivityIndicator style={{ margin: 50, backgroundColor: "#ABABABFF", 
	borderColor: "#00FF00FF", 
	borderWidth:50, 
      	shadowColor:'red',
	shadowOpacity: 1,
	shadowOffset:{width:10, height:10},
	position:'absolute',
	borderBottomColor:'#FABBCCFF',
	borderLeftColor:'#ABCDEFFF',
	borderRightColor:'#FFCCEEFF',
	borderStartColor:'#FACEFAFF',
	left:100, 
	top:100 }} 
	size={400} 
	color="#0000ffff" 
	animating = {false} 
	hidesWhenStopped = {false}/>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
