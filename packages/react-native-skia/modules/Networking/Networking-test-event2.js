import React, { useState, useEffect } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';


class SimpleViewApp extends React.Component {
  xhr: XMLHttpRequest;	
  constructor(props) {
    super(props);


  }
  

  loadTimeOutRequest() {

  //  this.xhr && this.xhr.abort();
const urls = [
  "https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-mp4-file.mp4","https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-mov-file.mov","https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-flv-file.flv", "https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-webm-file.webm","https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-ogv-file.ogv", "https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-3gp-file.3gp", "https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-3g2-file.3g2", "https://www.learningcontainer.com/wp-content/uploads/2020/02/Kalimba.mp3", "https://www.learningcontainer.com/wp-content/uploads/2020/02/Kalimba-online-audio-converter.com_-1.wav", "https://www.learningcontainer.com/wp-content/uploads/2020/02/Sample-FLAC-File.flac", "https://www.learningcontainer.com/wp-content/uploads/2020/02/Sample-OGG-File.ogg" , "https://www.learningcontainer.com/wp-content/uploads/2019/09/sample-pdf-file.pdf", "https://www.learningcontainer.com/wp-content/uploads/2019/09/sample-pdf-download-10-mb.pdf", "https://www.learningcontainer.com/wp-content/uploads/2019/09/sample-pdf-with-images.pdf"
];

    const xhr = this.xhr || new XMLHttpRequest();
    const xhr1 = new XMLHttpRequest();
    const xhr2 = new XMLHttpRequest();
    const xhr3 = new XMLHttpRequest();

    const xhr4 = new XMLHttpRequest();

    const xhr5 = new XMLHttpRequest();

    const xhr6 = new XMLHttpRequest();

    const xhr7 = new XMLHttpRequest();

    const xhr8 = new XMLHttpRequest();

    const xhr9 = new XMLHttpRequest();

    const xhr10 = new XMLHttpRequest();

    const xhr11 = new XMLHttpRequest();

    const xhr12 = new XMLHttpRequest();

    const xhr13 = new XMLHttpRequest();

    const xhr14 = new XMLHttpRequest();

    const xhr15 = new XMLHttpRequest();

    const xhr16 = new XMLHttpRequest();
    const xhr17 = new XMLHttpRequest();



   xhr.onreadystatechange = () => {
   var headers = xhr.getAllResponseHeaders();
   console.log('headers', headers);
   }

   xhr1.onreadystatechange = () => {
   var headers = xhr1.getAllResponseHeaders();
   }

   xhr.onerror = () => {
      console.log('Error ', xhr.responseText);
    };

   xhr.onloadend = function () {
   console.log("loadEnd",xhr.responseText);
   };

    xhr.onprogress = () => {
      console.log('onprogress ', xhr.responseText);
    };


    xhr.onabort = () => {
      console.log('abort ', xhr.responseText);
    };


    xhr.ontimeout = () => {
      console.log('Error ', xhr.responseText);
    };

    xhr.onload = () => {
      console.log('Response ', xhr.responseText);
      console.log('DONE: ', xhr.status);
    };

    xhr.open('GET','http://10.12.130.123:3001/');
    xhr1.open('GET','https://www.learningcontainer.com/wp-content/uploads/2020/05/sample-mp4-file.mp4');
    xhr2.open('GET',urls[9]);
    xhr3.open('GET',urls[10]);
    xhr4.open('GET',urls[2]);
    xhr5.open('GET',urls[3]);
    xhr6.open('GET',urls[4]);
    xhr7.open('GET',urls[5]);
    xhr8.open('GET',urls[6]);
    xhr9.open('GET',urls[7]);
    xhr10.open('GET',urls[8]);
    xhr11.open('GET',urls[9]);
    xhr12.open('GET',urls[10]);
    xhr13.open('GET',urls[11]);
    xhr14.open('GET',urls[0]);
    xhr15.open('GET',urls[1]);
    xhr16.open('GET',urls[2]);
    xhr17.open('GET',urls[3]);
    xhr.send("");
    xhr1.send("");

    xhr2.send("");
    xhr3.send("");
    xhr4.send("");
    xhr5.send("");
    xhr6.send("");
    xhr7.send("");
    xhr8.send("");
    xhr9.send("");
    xhr10.send("");
    xhr11.send("");
    xhr12.send("");
    xhr13.send("");
    xhr14.send("");
    xhr15.send("");
    xhr16.send("");
    xhr17.send("");
console.log('status ', xhr.status);



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
