import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style={{ margin:10 }}>
        <Text>Default textShadowOffset:</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:1, textShadowColor:'red' }}>textShadowRadius:1, textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
       <Text style={{ textShadowRadius:2, textShadowColor:'green' }}>textShadowRadius:2, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:3, textShadowColor:'blue' }}>textShadowRadius:3, textShadowColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:4, textShadowColor:'red' }}>textShadowRadius:4, textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:5, textShadowColor:'green' }}>textShadowRadius:5, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:6, textShadowColor:'blue' }}>textShadowRadius:6, textShadowColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:7, textShadowColor:'red' }}>textShadowRadius:7, textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:8, textShadowColor:'green' }}>textShadowRadius:8, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:9, textShadowColor:'blue' }}>textShadowRadius:9, textShadowColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:10, textShadowColor:'red' }}>textShadowRadius:10, textShadowColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:15, textShadowColor:'green' }}>textShadowRadius:15, textShadowColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textShadowRadius:20, textShadowColor:'blue' }}>textShadowRadius:20, textShadowColor:'blue'</Text>
      </View>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);


