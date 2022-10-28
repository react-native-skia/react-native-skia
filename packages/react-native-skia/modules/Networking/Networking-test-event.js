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

    xhr.onabort = () => {
      console.log('Abort', xhr.responseText);
    };

    xhr.onreadystatechange = function() {
       if(this.readyState == this.HEADERS_RECEIVED) {

        // Get the raw header string
         var headers = xhr.getAllResponseHeaders();
        console.log("Headers:",headers);
        // Convert the header string into an array
        // of individual headers
        var arr = headers.trim().split(/[\r\n]+/);

        // Create a map of header names to values
        var headerMap = {};
        arr.forEach(function (line) {
          var parts = line.split(': ');
          var header = parts.shift();
          var value = parts.join(': ');
          headerMap[header] = value;
        });
      }
    }

    //xhr.open('GET', 'https://reactnative.dev/'); // request to take 5 seconds to load
    //xhr.open('GET', 'https://pngimg.com/uploads/lion/lion_PNG23269.png'); // request to take 5 seconds to load
    xhr.open('GET', 'https://github.com/facebook/folly/tree/main/folly/io/async'); // request to take 5 seconds to load
    xhr.send();
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
