import * as React from 'react';
import { View, AppRegistry, Image, Text, TouchableOpacity } from 'react-native';

var imageURL = [
    'https://reactnative-examples.com/wp-content/uploads/2022/02/earth.jpg',
    'https://www.w3.org/2008/site/images/logo-w3c-screen-lg',
    'https://www.gstatic.com/webp/gallery3/1.sm.png',
    'https://reactnative.dev/img/tiny_logo.png',
]

const ImageSizeBlock = (idx) => {
  const [width, setWidth] = React.useState('');
  const [height, setHeight] = React.useState('');
  var index = idx ;

  function getSize() {
    console.log(index +" Image get Size call now");
    Image.getSize(imageURL[index], (Width, Height) => {
      console.log(index + " Image get Size received success");
      setWidth(Width);
      setHeight(Height);

    }, (errorMsg) => {
      console.log(errorMsg);

    });
    console.log(index + " Image get Size call done");
  }

  return (
    <View style={{flexDirection: 'row',margin:10}}>
      <TouchableOpacity style={{backgroundColor:'lightcyan'}} onPress={getSize}>
        <Text>{"Get Image Size"}</Text>
      </TouchableOpacity>
      <Text style={{fontSize: 28,textAlign: 'center', color: 'red',marginLeft:10}}>Image Width = {width}</Text>
      <Text style={{fontSize: 28,textAlign: 'center', color: 'red',marginLeft:10}}> , Height = {height}</Text>
    </View>
  )
}


const SimpleViewApp = React.Node = () => {

  return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: '#444' }}>
      <Image
        style={{ width: 250, height: 250 }}
        source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
      />
      <Text style={{ color: '#fff', fontSize: 36,
                     textAlign: 'center', marginTop: 32 }}>
        React Native loves Skia</Text>
      {ImageSizeBlock(0)}
      {ImageSizeBlock(1)}
      {ImageSizeBlock(2)}
      {ImageSizeBlock(3)}
    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

