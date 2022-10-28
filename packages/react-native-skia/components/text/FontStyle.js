import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{ margin: 10 }}>DEFAULT FONT STYLE</Text>
      <Text style={{ margin: 10, fontStyle: "normal" }}>FONT STYLE - NORMAL</Text>
      <Text style={{ margin: 10, fontStyle: "italic" }}>FONT STYLE - ITALIC</Text>
    </>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
