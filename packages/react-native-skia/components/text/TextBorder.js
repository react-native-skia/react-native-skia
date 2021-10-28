import * as React from 'react';
import { AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:5}}>margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:5</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lightcyan', borderColor:'red', borderStyle:'solid', borderWidth:5}}>margin :10, height:50, width:600, backgroundColor:'lightcyan', borderColor:'red', borderStyle:'solid', borderWidth:5</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderColor:'green', borderStyle:'dashed', borderRadius:5, borderWidth:5}}>margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderColor:'green', borderStyle:'dashed', borderRadius:5, borderWidth:5</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lightcyan', borderColor:'blue', borderStyle:'dotted', borderRadius:15, borderWidth:10}}>margin :10, height:50, width:600, backgroundColor:'lightcyan', borderColor:'blue', borderStyle:'dotted', borderRadius:15, borderWidth:10</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:0}}>margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:0</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lightcyan', borderWidth:-10}}>margin :10, height:50, width:600, backgroundColor:'lightcyan', borderWidth:-10</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:5, borderRadius:0}}>margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderWidth:5, borderRadius:0</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lightcyan', borderWidth:5, borderRadius:-10}}>margin :10, height:50, width:600, backgroundColor:'lightcyan', borderWidth:5, borderRadius:-10</Text>
      <Text style={{margin :10, height:100, width:600, backgroundColor:'lemonchiffon', borderLeftWidth:5, borderLeftColor:'red', borderTopWidth:10, borderTopColor:'blue', borderRightWidth:20, borderRightColor:'green', borderBottomWidth:30, borderBottomColor:'grey'}}>margin :10, height:100, width:600, backgroundColor:'lemonchiffon', borderLeftWidth:5, borderLeftColor:'red', borderTopWidth:10, borderTopColor:'blue', borderRightWidth:20, borderRightColor:'green', borderBottomWidth:30, borderBottomColor:'grey'</Text>
      <Text style={{margin :10, height:100, width:600, backgroundColor:'lightcyan', borderWidth:10, borderTopLeftRadius:5, borderTopRightRadius:20, borderBottomRightRadius:40, borderBottomLeftRadius:60}}>margin :10, height:100, width:600, backgroundColor:'lightcyan', borderWidth:10, borderTopLeftRadius:5, borderTopRightRadius:20, borderBottomRightRadius:40, borderBottomLeftRadius:60</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderStartWidth:10,borderStartColor:'red', borderEndWidth:40, borderEndColor:'blue'}}>margin :10, height:50, width:600, backgroundColor:'lemonchiffon', borderStartWidth:10,borderStartColor:'red', borderEndWidth:40, borderEndColor:'blue'</Text>
      <Text style={{margin :10, height:50, width:600, backgroundColor:'lightcyan', borderStartWidth:10,borderStartColor:'red', borderEndWidth:40, borderEndColor:'blue', borderTopStartRadius:10, borderTopEndRadius:10, borderBottomEndRadius:10, borderBottomStartRadius:10}}>margin :10, height:50, width:600, backgroundColor:'lightcyan', borderStartWidth:10,borderStartColor:'red', borderEndWidth:40, borderEndColor:'blue', borderTopStartRadius:10, borderTopEndRadius:10, borderBottomEndRadius:10, borderBottomStartRadius:10</Text>
      <Text style={{marginTop :10, fontSize:30, backgroundColor:'lemonchiffon', borderWidth:5}}>marginTop :10, fontSize:30, backgroundColor:'lemonchiffon', borderWidth:5</Text>
      <Text style={{marginTop :10, fontSize:30, backgroundColor:'lightcyan', borderWidth:5, borderRadius:30, borderColor:'red'}}>marginTop :10, fontSize:30, backgroundColor:'lightcyan', borderWidth:5, borderRadius:30, borderColor:'red'</Text>
    </>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);