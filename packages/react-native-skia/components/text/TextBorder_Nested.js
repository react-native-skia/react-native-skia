import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{height:200, width:200, backgroundColor:'red', borderColor:'green', borderStyle:'solid', borderRadius:20, borderWidth:5}}>height:200, width:200, backgroundColor:'red', borderColor:'green', borderStyle:'solid', borderRadius:20, borderWidth:5</Text>
      <Text style={{height:300, width:300, backgroundColor:'blue', borderColor:'red', borderStyle:'dotted', borderRadius:20, borderWidth:10}}>1 - height:300, width:300, backgroundColor:'blue', borderColor:'red', borderStyle:'dotted', borderRadius:20, borderWidth:10 
        <Text style={{height:250, width:250, backgroundColor:'grey', borderColor:'green', borderStyle:'solid', borderRadius:5, borderWidth:4}}>2 - height:250, width:250, backgroundColor:'grey', borderColor:'green', borderStyle:'solid', borderRadius:5, borderWidth:4</Text>
        <Text style={{height:200, width:200, backgroundColor:'pink', borderColor:'green', borderStyle:'solid', borderRadius:5, borderWidth:4}}>3 - height:200, width:200, backgroundColor:'pink', borderColor:'green', borderStyle:'solid', borderRadius:5, borderWidth:4</Text>
      </Text>
      <Text style={{height:200, width:200, backgroundColor:'cyan', borderColor:'yellow', borderStyle:'solid', borderWidth:10}}>height:100, width:100, backgroundColor:'cyan', borderColor:'yellow', borderStyle:'solid', borderWidth:10</Text>
      
    </>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
