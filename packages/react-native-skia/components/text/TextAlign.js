import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
        <Text style= {{textAlign:'auto'}}>textAlign:'auto'</Text>
        <Text style= {{textAlign:'left'}}>textAlign:'left'</Text>
        <Text style= {{textAlign:'center'}}>textAlign:'center'</Text>
        <Text style= {{textAlign:'right'}}>textAlign:'right'</Text>
        <Text style= {{textAlign:'justify'}}>textAlign:'justify'</Text>      
    </>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
