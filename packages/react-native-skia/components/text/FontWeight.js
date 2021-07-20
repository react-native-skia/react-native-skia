import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{ margin: 10 }}>DEFAULT FONT WEIGHT</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: 'normal' }}>FONT WEIGHT - NORMAL</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: 'bold' }}>FONT WEIGHT - BOLD</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '100' }}>FONT WEIGHT - 100</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '200' }}>FONT WEIGHT - 200</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '300' }}>FONT WEIGHT - 300</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '400' }}>FONT WEIGHT - 400</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '500' }}>FONT WEIGHT - 500</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '600' }}>FONT WEIGHT - 600</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '700' }}>FONT WEIGHT - 700</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '800' }}>FONT WEIGHT - 800</Text>
      <Text style={{ margin: 10, fontSize: 30,fontWeight: '900' }}>FONT WEIGHT - 900</Text>
    </>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
