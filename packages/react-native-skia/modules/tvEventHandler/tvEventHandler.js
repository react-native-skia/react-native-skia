import React, { useState, useEffect } from 'react';
import { View, AppRegistry, Image, Text, TVEventHandler } from 'react-native';

console.log('TVEventHandler: ', TVEventHandler);

class SimpleViewApp extends React.Component {
  constructor(props) {
    super(props);

    this._tvEventHandler = null;
    this.state = {
      bgColor: '#FFFFFF'
    }

  }
  
  _enableTVEventHandler() {
    let that = this;
    this._tvEventHandler = new TVEventHandler();
    console.log('APP: TVEventHandler ',this._tvEventHandler);
    this._tvEventHandler.enable(this, function(cmp, evt) {
      console.log('APP: TV Key event received: ', evt);

      if (evt && evt.eventType === 'right') {
        that.setState({bgColor: '#FF0000'})
      } else if(evt && evt.eventType === 'up') {
        that.setState({bgColor: '#00FF00'})
      } else if(evt && evt.eventType === 'left') {
        that.setState({bgColor: '#0000FF'})
      } else if(evt && evt.eventType === 'down') {
        that.setState({bgColor: '#00FFFF'})
      } else if(evt && evt.eventType === 'select') {
        that.setState({bgColor: '#FFFF00'})
      }
    });
  }

  _disableTVEventHandler() {
    if (this._tvEventHandler) {
      this._tvEventHandler.disable();
      delete this._tvEventHandler;
    }
  }

  componentDidMount() {
    this._enableTVEventHandler();
  }

  componentWillUnmount() {
    this._disableTVEventHandler();
  }

  render() {
    console.log('bgColor: ', this.state.bgColor);
    return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: this.state.bgColor }}
      onLayout={() => console.log('onLayout')}>
      <Image
        style={{ width: 512, height: 512 }}
        source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
        onLoad={() => console.log('onLoad')}
      />
      <Text style={{ color: '#fff', fontSize: 36,
                    textAlign: 'center', marginTop: 32 }}>
        React Native loves Skia
      </Text>
    </View>
    );
  }
}

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
