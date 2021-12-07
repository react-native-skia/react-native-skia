'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  View,
  Text,
  Image,
  AppRegistry,
} = require('react-native');


class ScrollViewSimpleExample extends React.Component<{...}> {

  render(): React.Node {

    const verticalScrollView = (
      <ScrollView style={styles.verticalScrollView} horizontal={true}>
      <Image
        style={{ width: 300, height: 300 ,margin:10 ,resizeMode:'stretch'}}
        source={require('react-native-skia/demo2/images/poster1.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster2.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster3.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster4.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster5.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster6.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster7.jpg')}
      />
      <Image
        style={{ width: 300, height: 300 ,margin:10}}
        source={require('react-native-skia/demo2/images/poster8.jpg')}
      />
      </ScrollView>
    );

    return verticalScrollView;
  }
}

const styles = StyleSheet.create({
  verticalScrollView: {
    margin: 10,
    backgroundColor : 'pink',
    borderWidth: 5,
    borderColor: 'black',
    width:500,
    height:500
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
