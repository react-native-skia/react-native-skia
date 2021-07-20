import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{ margin: 10 }}>DEFAULT FONT FAMILY</Text>
      <Text style={{ margin: 10, fontFamily: "Times New Roman" }}>FONT FAMILY - Times New Roman</Text>
      <Text style={{ margin: 10, fontFamily: "Gotham" }}>FONT FAMILY - Gotham</Text>
      <Text style={{ margin: 10, fontFamily: "Futura" }}>FONT FAMILY - Futura</Text>
      <Text style={{ margin: 10, fontFamily: "Calibri" }}>FONT FAMILY - Calibri</Text>
      <Text style={{ margin: 10, fontFamily: "Arial" }}>FONT FAMILY - Arial</Text>
    </>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
