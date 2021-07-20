import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{ margin: 10 }}>DEFAULT FONT SIZE</Text>
      <Text style={{ margin: 10, fontSize: 15 }}>FONT SIZE - 15</Text>
      <Text style={{ margin: 10, fontSize: 50 }}>FONT SIZE - 50</Text>
    </>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
