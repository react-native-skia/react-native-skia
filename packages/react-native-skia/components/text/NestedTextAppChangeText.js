import * as React from 'react';
import {useState} from 'react';
import { View, AppRegistry, Image, Text , Platform} from 'react-native';

const SimpleViewApp = React.Node = () => {
  const [changeText, toggleText] = useState(true);
  const timer = setTimeout(()=>{
            toggleText(!changeText);
        }, 40000);

  const renderText = () => {
      if(changeText) {
         return (
           <Text style={{ fontSize:30 , color : 'red'}}>
               Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2 Text2
           </Text>)

      } else {
         return (
           <Text style={{ fontSize:30 , color :'red'}}>
               Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21 Text21
              <Text style={{fontSize:30 , color :'red'}}>
                  Text22 Text22
              </Text>
           </Text>)
      }

  }

  return (
    <View style={{ flex: 1,backgroundColor: 'blue' }}>
       <Text style={{ fontColor : 'red', marginTop: 50, marginLeft: 50, marginRight: 50}} >
          <Text style={{ fontSize: 30 , color:'white'}}>
              Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1 Text1
          </Text>
          {renderText()}
          <Text style={{fontSize:30 , color:'white'}}>
              Text3 Text3 Text3 Text3
          </Text>
          <Text style={{fontSize:30 , color:'white'}}>
              Text4
          </Text>
        </Text>

        <Text style={{fontSize:20 ,color : 'gold',marginTop:50,marginLeft:50,marginRight:50}} >
            PARAGRAPH TEST : A React component for displaying text.Text supports nesting, styling, and touch handling.The Text element is unique relative to layout: everything inside is no longer using the Flexbox layout but using text layout. This means that elements inside of a Text are no longer rectangles, but wrap when they see the end of the line.
        </Text>
    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
