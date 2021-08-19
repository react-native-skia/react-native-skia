import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style={{ margin:10, backgroundColor: 'red' }}>
        <Text>0-Default lineHeight</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'red' }}>
        <Text style={{ lineHeight:20 }}>1- lineHeight:20</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'green' }}>
       <Text style={{ lineHeight:30 }}>2-- lineHeight:30</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'blue' }}>
        <Text style={{ lineHeight:40 }}>3-- lineHeight:40</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'red' }}>
        <Text style={{ lineHeight:5 }}>4-- lineHeight:5</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'green' }}>
        <Text style={{ lineHeight:12.5 }}>5-- lineHeight:12.5</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'blue' }}>
        <Text style={{ lineHeight:14 }}>6-- lineHeight:14</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'red' }}>
        <Text style={{ lineHeight:33.3 }}>7- lineHeight:33.3</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'green' }}>
        <Text style={{ lineHeight:-10 }}>8-- lineHeight:-10</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'blue' }}>
        <Text style={{ lineHeight:-5 }}>9-- lineHeight:-5</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'red' }}>
        <Text style={{ lineHeight:-20 }}>10-- lineHeight:-20</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'green' }}>
        <Text style={{ lineHeight:10 }}>11-- lineHeight:10</Text>
      </View>
      <View style={{ margin:10, backgroundColor: 'blue' }}>
        <Text style={{ lineHeight:20 }}>12-- lineHeight:20</Text>
      </View>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
