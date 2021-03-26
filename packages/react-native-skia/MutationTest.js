import * as React from 'react';
import { useEffect, useState } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  const [color, setColor] = useState('#444');

  useEffect(() => {
    const timeout = setTimeout(() => {
      if (color === '#444') {
        setColor('#00ff88');
      } else {
        setColor('#444');
      }
    }, 2000);

    return () => {
      clearTimeout(timeout);
    };
  }, [color]);
  return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: color }}>
      <Image
        style={{ width: 512, height: 512 }}
        source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
      />
      {color === '#00ff88' && (
        <View style={{ width: 100, height: 100, backgroundColor: 'blue' }} />
      )}
      <Text style={{ color: '#fff', fontSize: 36,
                     textAlign: 'center', marginTop: 32 }}>
        React Native loves Skia</Text>

    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
