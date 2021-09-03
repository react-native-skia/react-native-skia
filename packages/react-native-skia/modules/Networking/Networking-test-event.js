import React, { useState, useEffect } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';


class SimpleViewApp extends React.Component {
  xhr: XMLHttpRequest;	
  constructor(props) {
    super(props);


  }
  

  loadTimeOutRequest() {
    this.xhr && this.xhr.abort();

    const xhr = this.xhr || new XMLHttpRequest();

    xhr.onerror = () => {
      console.log('Error ', xhr.responseText);
    };

    xhr.ontimeout = () => {
      console.log('Error ', xhr.responseText);
    };

    xhr.onload = () => {
      console.log('Response ', xhr.responseText);
    };
	  xhr.open('POST', 'https://jsnplaceholder.typicode.com/'); // request to take 5 seconds to load
//    xhr.timeout = 200; // request times out in 2 seconds
    xhr.send("1");
    this.xhr = xhr;

  }


  componentDidMount() {
    this.loadTimeOutRequest();
    
  }

  componentWillUnmount() {
  }

  render() {
    return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColoe: 'blue'}}>
      <Image
        style={{ width: 512, height: 512 }}
        source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
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
