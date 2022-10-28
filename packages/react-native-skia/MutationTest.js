import * as React from 'react';
import { useEffect, useState } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  const [color, setColor] = useState('#444');
  const [toggleui, setToggleUi] = useState(false);

  useEffect(() => {
    const timeout = setTimeout(() => {
      if (color === '#444') {
        setColor('#00ff88');
      } else {
        setColor('#444');
      }
      setToggleUi(!toggleui);      
    }, 2000);

    return () => {
      clearTimeout(timeout);
    };
  }, [color]);

  const showToggleUi= () => {
      if(toggleui) {
          return (
	      <View style={{width:250,height:250}}>
                    <Text style={{ color: '#fff', fontSize: 36,
                        textAlign: 'center', marginTop: 32 }}>
                     React Native loves Skia</Text>
	      </View>	  
	  );
      } else {
          return (
              <Image
                 style={{ width: 512, height: 512 }}
                 source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
              />
	  );

      }
  }

  return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: 'grey' }}>
      {color === '#00ff88' && (
        <View style={{ width: 100, height: 100, backgroundColor: 'blue' }} />
      )}
      {showToggleUi()}	  

    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
