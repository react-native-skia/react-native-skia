import * as React from 'react';
import { Text, AppRegistry } from 'react-native';

const SimpleViewApp = React.Node = () => {

var url = "https://reqbin.com/echo/post/json";

var xhr = new XMLHttpRequest();
xhr.open("POST", url);

xhr.setRequestHeader("Accept", "application/json");
xhr.setRequestHeader("Content-Type", "application/json");

xhr.onreadystatechange = function () {
   if (xhr.readyState === 4) {
      console.log(xhr.status);
      console.log(xhr.responseText);
   }};

var data = `{
  "Id": 78912,
  "Customer": "Jason Sweet",
  "Quantity": 1,
  "Price": 18.00
}`;

xhr.send(data);


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

