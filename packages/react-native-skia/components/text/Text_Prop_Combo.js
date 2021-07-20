import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{ color: 'black', fontSize: 30, margin: 10, fontStyle: "normal", fontWeight: "normal", fontFamily: "Courier New" }}>color: 'black', fontSize: 30, margin: 10, fontStyle: "normal", fontWeight: "normal", fontFamily: "Courier New"</Text>
      <Text style={{ color: 'blue', fontSize: 20, margin: 10, fontStyle: "normal", fontWeight: "bold", fontFamily: "Arial" }}>color: 'blue', fontSize: 20, margin: 10, fontStyle: "normal", fontWeight: "bold", fontFamily: "Arial"</Text>
      <Text style={{ color: 'red', fontSize: 30, margin: 10, fontStyle: "italic", fontWeight: "normal", fontFamily: "Roboto" }}>color: 'red', fontSize: 30, margin: 10, fontStyle: "italic", fontWeight: "normal", fontFamily: "Roboto"</Text>
      <Text style={{ color: 'green', fontSize: 40, margin: 10, fontStyle: "italic", fontWeight: "bold", fontFamily: "monospace" }}>color: 'green', fontSize: 40, margin: 10, fontStyle: "italic", fontWeight: "bold", fontFamily: "monospace"</Text>
      <Text style={{ color: 'brown', fontSize: 20, margin: 10, fontStyle: "normal", fontWeight: "900", fontFamily: "sans-serif" }}>color: 'brown', fontSize: 20, margin: 10, fontStyle: "normal", fontWeight: "900", fontFamily: "sans-serif"</Text>
    </>
  );
};
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);