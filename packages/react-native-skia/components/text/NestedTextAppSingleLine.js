import * as React from 'react';
import { View, AppRegistry, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {

  return (
    <View style={{ flex: 1, backgroundColor: 'pink' }}>
        <View style={{backgroundColor:'lightblue'}} >
            <Text style={{ marginTop: 50, textAlign: 'center', marginLeft: 50, marginRight: 50}}>
               NESTED TEXT : SINGLE LINE TEXTS - DEFAULT WIDTH
            </Text>
        </View>

        <Text style={{ color:'white', textAlign: 'center' , marginTop: 50, marginLeft: 50 ,marginRight: 50 }}>
           <Text style={{ fontSize: 10 , color:'blue'}}>
               TEXT1 TEXT1 TEXT1 TEXT1
           </Text>
           <Text style={{ fontSize: 10 , color: 'red'}}>
               TEXT2 TEXT2 TEXT2 TEXT2
           </Text>
           <Text style={{ fontSize: 16 , color:'green'}}>
               tEXT3 TEXT3 TEXT3 TEXT3
           </Text>
           <Text style={{ fontSize: 20, color: 'fuchsia'}}>
               4TEXT4 TEXT4 TEXT4 TEXT4
           </Text>
           <Text style={{ fontSize: 30, color:'indianred'}}>
               !TEXT5 TEXT5 TEXT5 TEXT5
           </Text>
           <Text style={{ fontSize: 36, color: 'purple'}}>
               gTEXT6 TEXT6 TEXT6 TEXT6
           </Text>
      </Text>

      <View style={{backgroundColor:'lightblue'}} >
         <Text style={{ marginTop: 50, textAlign: 'center',marginLeft: 50, marginRight: 50}}>
            NESTED TEXT : SINGLE LINE TEXTS - FIXED WIDTH 500
         </Text>
      </View>
      <Text style={{ width:500 , marginTop: 50, marginLeft: 50 ,marginRight: 50 }}>
         <Text style={{ fontSize: 10 , color: 'blue'}}>
            TEXT1 TEXT1 TEXT1 TEXT1
         </Text>
         <Text style={{ fontSize: 10, color: 'red'}}>
            TEXT2 TEXT2 TEXT2 TEXT2
         </Text>
         <Text style={{ fontSize: 16, color: 'green'}}>
            tEXT3 TEXT3 TEXT3 TEXT3
         </Text>
         <Text style={{ fontSize: 20, color: 'fuchsia'}}>
            4TEXT4 TEXT4 TEXT4 TEXT4
         </Text>
         <Text style={{ fontSize: 30, color: 'indianred'}}>
            !TEXT5 TEXT5 TEXT5 TEXT5
         </Text>
         <Text style={{ fontSize: 36, color: 'purple'}}>
            gTEXT6 TEXT6 TEXT6 TEXT6
         </Text>
      </Text>

    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
