import * as React from 'react';
import {useState} from 'react';
import { AppRegistry, View,Text,ScrollView } from 'react-native';

const NUM_ITEMS = 25;

const  ScrollViewSimpleExample = React.Node = () => {
  const [numItems,setNumItems] = useState(NUM_ITEMS);
  const [bgColor,setBgColor] = useState('yellow');
  const [count,setCount] = useState(0);	
  const timer = setTimeout(()=>{
     setCount(count+1)
     if(count == 1) {
         setNumItems(25);
         setBgColor('blue');
     } else if (count == 2) {
         setBgColor('green');
         setNumItems(20);
     } else if ( count == 3) {
         setBgColor('yellow');
	 setCount(0);    
     } 
  }, 15000);

  const items = [];
  for (let i = 0; i < numItems; i++) {
      items[i] = (
          <View key={i} style={{ backgroundColor: bgColor,
                                 alignItems: 'center',
                                 borderRadius: 5,
                                 borderWidth: 5,
                                 borderColor: '#a52a2a',
                                 padding: 30,
                                 margin: 5,}}>
            <Text>{'Item ' + i}</Text>
          </View>
        );
  }


  const verticalScrollView = (
      <ScrollView style={{margin: 10,backgroundColor : 'pink',borderWidth: 5,borderColor: 'black'}}>
          {items}
          <View style={{ backgroundColor: 'red',
                                 alignItems: 'center',
                                 borderRadius: 5,
                                 borderWidth: 5,
                                 borderColor: '#a52a2a',
                                 padding: 30,
                                 margin: 5,
                                 width: 200,
                                 height: 300,}}>
         </View>
         <View style={{ backgroundColor: 'green',
                                 alignItems: 'center',
                                 borderRadius: 5,
                                 borderWidth: 5,
                                 borderColor: '#a52a2a',
                                 padding: 30,
                                 margin: 5,
                                 width: 200,
                                 height: 300,}}>
         </View>

      </ScrollView>
  );


  return verticalScrollView;
}


AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);