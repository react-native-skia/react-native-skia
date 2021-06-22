import * as React from 'react';
import { View, AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {

  return (
    <View style={{ flex: 1, backgroundColor: 'pink' }}>
      <Text style={{ marginTop: 50, textAlign: 'center',marginLeft: 50, marginRight: 50}}>
          NESTED TEXT : MULTI & SINGLE LINE TEXTS COMBO- DEFAULT WIDTH
      </Text>

      <View style={{backgroundColor:'khaki'}} >
         <Text style={{marginTop: 50, marginLeft: 50 ,marginRight: 50 , color: 'blue'}}>
              TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1
              <Text style={{ fontSize: 10 , color:'red'}}>
                  TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2
              </Text>
              <Text style={{ fontSize: 16 , color: 'green'}}>
                  tEXT3
              </Text>
              <Text style={{ fontSize: 30 , color:'fuchsia'}}>
                  !TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4
              </Text>
              <Text style={{ fontSize: 36 , color : 'purple'}}>
                  gTEXT5 TEXT5 TEXT5
              </Text>
          </Text>
      </View>

      <Text style={{ marginTop: 50, textAlign: 'center',marginLeft: 50, marginRight: 50}}>
          NESTED TEXT : MULTI & SINGLE LINE TEXTS COMBO- FIXED WIDTH 600
      </Text>
      <Text style={{width:600, marginTop: 50, marginLeft: 50 ,marginRight: 50 , color:'blue'}}>
          TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1 TEXT1
          <Text style={{ fontSize: 10 , color:'red'}}>
               TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2 TEXT2
          </Text>
          <Text style={{ fontSize: 16 , color:'green'}}>
               tEXT3
          </Text>
          <Text style={{ fontSize: 30, color:'fuchsia'}}>
               !TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4 TEXT4
          </Text>
          <Text style={{ fontSize: 36, color:'purple'}}>
               gTEXT5 TEXT5 TEXT5
          </Text>
      </Text>
    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
