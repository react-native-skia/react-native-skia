import * as React from 'react';
import {useState} from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {

  let [bgColor,setBgColor] = useState('yellow');
  let [showChild,toggleChild] = useState(false);

  setTimeout(()=>{
      toggleChild(!showChild);
  },2000);

  setTimeout(()=>{
      if(bgColor == 'yellow') setBgColor('orange');
      else setBgColor('yellow');
  },5000);

  let childOverflow = () => {
     if(showChild) {
       return(
         <View style={{backgroundColor:"pink",borderWidth:5,borderColor:'red',width:300,height:300,position:'absolute',top:550,left:100}}>
           <View style={{backgroundColor:"grey",width:300,height:200,marginLeft:200,marginTop:50,overflow:"hidden"}}>
             <View style={{backgroundColor:"lightcyan",width:100,height:200,marginLeft:50}}>
             </View>
             <View style={{backgroundColor:"lightcyan",width:100,height:200,marginLeft:50}}>
             </View>
             <View style={{backgroundColor:"lightcyan",width:100,height:200,marginLeft:50}}>
             </View>
           </View>
         </View>
       )
     }
  }

  return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: '#444' }}>
      <Text style={{position:"absolute",top:10,left:50}}>{"Overflow:Visible"}</Text>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:50,left:50}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:50,left:200}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:200,left:50}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:200,left:200}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:5,borderColor:'red',width:150,height:150,position:'absolute',top:100,left:100,overflow:"visible"}}>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:5,borderColor:'red',width:100,height:100,position:'absolute',top:150,left:100}}>
      </View>
      <Text style={{position:"absolute",top:10,left:350}}>{"Overflow:Hidden"}</Text>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:50,left:350}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:50,left:500}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:200,left:350}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:2,borderColor:'black',width:100,height:100,position:'absolute',top:200,left:500}}>
      </View>
      <View style={{backgroundColor:bgColor,borderWidth:5,borderColor:'red',width:150,height:150,position:'absolute',top:100,left:400,overflow:"hidden"}}>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
         <View style={{backgroundColor:"green",width:100,height:50,margin:5}}>
         </View>
      </View>
      <Text style={{position:"absolute",top:500,left:100}}>{"Child Overflow:Hidden"}</Text>
      {childOverflow()}
    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
