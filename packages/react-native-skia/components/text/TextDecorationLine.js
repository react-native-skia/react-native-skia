import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style={{ margin:10 }}>
        <Text>Default textDecorationLine:</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textDecorationLine:'underline', textDecorationColor:'green' }}>textDecorationLine:'underline', textDecorationColor:'green'</Text>
      </View>
      <View style={{ margin:10 }}>
      <Text style={{ textDecorationLine:'line-through', textDecorationColor:'blue' }}>textDecorationLine:'line-through', textDecorationColor:'blue'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textDecorationLine:'underline line-through', textDecorationColor:'red' }}>textDecorationLine:'underline line-through', textDecorationColor:'red'</Text>
      </View>
      <View style={{ margin:10 }}>
        <Text style={{ textDecorationLine:'none', textDecorationColor:'blue' }}>textDecorationLine:'none', textDecorationColor:'blue'</Text>
      </View>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

