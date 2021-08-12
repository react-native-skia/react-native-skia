import * as React from 'react';
import { Text, AppRegistry } from 'react-native';

const SimpleViewApp = React.Node = () => {

var url = "https://reqbin.com/echo/post/json";

var xhr = new XMLHttpRequest();
request.open('GET', 'https://www.google.co.in');
request.setRequestHeader('Content-Type', 'text/plain');
request.setRequestHeader('User-Agent',Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko');
request.setRequestHeader('Accept-Language', 'en');
request.setRequestHeader('Accept','text/plain');

xhr.onreadystatechange = function () {
   if (xhr.readyState === 4) {
      console.log(xhr.status);
      console.log(xhr.responseText);
   }};


xhr.send();


request.send();
  return (
    <>
   <Text style={{ color: '#fff', fontSize: 36,
                     textAlign: 'center', marginTop: 32 }}>
        React Native loves Skia</Text>
          </>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
