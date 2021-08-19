import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style={{ margin:10 }}>
        <Text>Default textShadowOffset:</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:2,height:2}, textShadowColor:'red' }}>textShadowOffset: width:2,height:2 , textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
       <Text style={{ textShadowOffset:{width:10,height:8}, textShadowColor:'green' }}>textShadowOffset: width:5,height:8, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:20,height:4}, textShadowColor:'blue' }}>textShadowOffset: width:10,height:4, textShadowColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:-15,height:-5}, textShadowColor:'red' }}>textShadowOffset: width:-5,height:-5, textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:9.5,height:-7.6}, textShadowColor:'green' }}>textShadowOffset: width:3.5,height:-7.6, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:16.6,height:-5.5}, textShadowColor:'blue' }}>textShadowOffset: width:6.6,height:-5.5, textShadowColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowOffset:{width:2,height:2}, textShadowColor:'red' }}>textShadowOffset: width:2,height:2, textShadowColor:'red'</Text>
      </View>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

