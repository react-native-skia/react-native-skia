import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
        <Text style= {{textAlign:'auto'}}>textAlign:'auto'</Text>
        <Text style= {{textAlign:'left'}}>textAlign:'auto'</Text>
        <Text style= {{textAlign:'center'}}>textAlign:'auto'</Text>
        <Text style= {{textAlign:'right'}}>textAlign:'auto'</Text>
        <Text style= {{textAlign:'justify'}}>textAlign:'auto'</Text>      
    </>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
