import * as React from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View style={{width:600, height:600, borderWidth:20, borderColor:'black'}}>
      <Text style={{fontSize:24, backgroundColor:"brown"}}>React Native loves Skia</Text>
      <Image style={{width:30, height:30, backgroundColor:"green"}} source={require('react-native/Libraries/NewAppScreen/components/logo.png')}/>
      <View style={{borderWidth:10, borderColor:'grey'}}>
          <Text style={{fontSize:20, backgroundColor:"pink"}}>React Native loves Skia</Text>
          <Image style={{width:20, height:20, backgroundColor:"brown"}} source={require('react-native/Libraries/NewAppScreen/components/logo.png')}/>
          <Text style={{fontSize:20, backgroundColor:"red"}}>React Native<Text style={{fontSize:20, backgroundColor:"orange"}}>React Native loves Skia</Text>loves Skia</Text>
      </View>
      <View style={{borderWidth:10, borderColor:'green'}}>
        <Text style={{fontSize:24, backgroundColor:"blue"}}>React Native 
          <Text style={{fontSize:24, backgroundColor:"cyan"}}>React Native loves Skia</Text>
        </Text>
      </View>
    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
v
