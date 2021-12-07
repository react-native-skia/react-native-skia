import * as React from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
   <>
      <View
          style={{
                  flex:0.5,
                  flexDirection:'row',
          }}>

         <View
             style={{  width: 300,
                       height: 300,
                       borderLeftWidth:10,
                       borderBottomWidth:10,
                       borderColor:'black',
                       shadowColor:'aqua',
                       shadowOpacity:1,
                       shadowOffset:{
                                     width:10,
                                     height:10
                       },
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,
                         shadowColor: "yellow",
                         shadowRadius: 1,
                         shadowOpacity: 1,
                         borderWidth:5,
                         borderColor:'#ff0000',
                         shadowOffset: {
                                       width: 10,
                                       height: 10}
            }}/>
        </View>
        <View
            style={{ width: 300,
                     height: 300,
                     marginLeft:30,
                     borderLeftWidth:10,
                     borderBottomWidth:10,
                     borderColor:'black',
                     shadowColor:'cyan',
                     shadowOpacity:1,
                     backgroundColor:'cornsilk',
                     shadowOffset:{
                                   width:10,
                                   height:10
                     },
            }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200,
                         height: 200,
                         shadowColor:"yellow",
                         shadowRadius:1,
                         shadowOpacity:1,
                         borderWidth:5,
                         backgroundColor:'#ffe4e1',
                         borderColor:'red',
                         shadowOffset: {
                                         width: 10,
                                         height: 10}
               }}/>
        </View>
    </View>

    <View
        style={{
            marginTop:300,
            width: 500,
            height: 300,
        }}>
        <Text
            style={{
                    color: '#000000',
                    fontSize: 40,
                    backgroundColor:"mistyrose",
                    shadowColor: "red",
                    shadowRadius: 0,
                    shadowOpacity: 1,
                    shadowOffset: {
                                  width: 2,
                                  height: 2},
                    textShadowColor:'green',
                    textShadowOffset:{
                                   width:2,
                                   height:2
                    },
                    textAlign:'center',
            }}>
                   React Native loves Skia
        </Text>
        <Text
            style={{
                    color: '#000000',
                    fontSize: 40,
                    shadowColor: "red",
                    shadowRadius: 0,
                    shadowOpacity: 1,
                    shadowOffset: {
                                  width: 4,
                                  height: 4},
                    textShadowColor:'green',
                    textShadowOffset:{
                                   width:4,
                                   height:4
                    },
                    textAlign: 'center',
                    marginTop: 50
            }}>
                  React Native loves Skia
        </Text>
        <Text
            style={{
                    color: '#000000',
                    fontSize: 40,
                    shadowColor: "red",
                    shadowRadius: 0,
                    shadowOpacity: 1,
                    shadowOffset: {
                                  width: 10,
                                  height: 10},
                    textShadowColor:'green',
                    textShadowOffset:{
                                   width:5,
                                   height:5
                    },
                    textAlign:'center',
                    marginTop:32
            }}>
                  React Native loves Skia
        </Text>
    </View>
  </>
  );
};
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
