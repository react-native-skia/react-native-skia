import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style = {{backgroundColor:'yellow', width:300, height:300}}>
        <Text style= {{textAlignVertical: 'auto', width:300, height:300}}>textAlignVertical: 'auto', width:300, height:300</Text>
      </View>
      <View style = {{backgroundColor:'cyan', width:300, heigh:300}}>
        <Text style= {{textAlignVertical: 'top', width:300, height:300}}>textAlignVertical: 'top', width:300, height:300</Text>
      </View>
      <View style = {{backgroundColor:'yellow', width:300, height:300}}>
        <Text style= {{textAlignVertical: 'center', width:300, height:300}}>textAlignVertical: 'center', width:300, height:300</Text>
      </View>
      <View style = {{backgroundColor:'cyan', width:300, height:300}}>
        <Text style= {{textAlignVertical: 'bottom', width:300, height:300}}>textAlignVertical: 'bottom', width:300, height:300</Text>
      </View>
  </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
