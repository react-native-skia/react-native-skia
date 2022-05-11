import React, { Component } from "react";
import { Text, AppRegistry, StyleSheet, View } from "react-native";

function makeid(length) {
    var result           = '';
    var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    var charactersLength = characters.length;
    for ( var i = 0; i < length; i++ ) {
      result += characters.charAt(Math.floor(Math.random() * 
 charactersLength));
   }
   return result;
}

class wsClient extends Component{
  constructor(props){
    super(props)
    //console.log("-----this.onmessage " + this.onMessage)
    console.log("------in wsClient : " + JSON.stringify(props.uri))
    this.ws = new WebSocket(props.uri);
    this.ws.onopen = () => {
      console.log("onopen---- client:", this.props.clientName)
      // connection opened
      this.ws.send(this.props.clientName+'-says-hello'); // send a message
    };

    this.ws.onmessage = (e) => {
      console.log("onmessage---- client:", this.props.clientName + " msg:" + (JSON.stringify(e.data)));
    }

    this.ws.onerror = (e) => {
      console.log("onerror---- client:", this.props.clientName)
      // an error occurred
      console.log(e.message);
    };

    this.ws.onclose = (e) => {
      console.log("onclose---- client:", this.props.clientName)
      // connection closed
      console.log(e.code, e.reason);
    };
  }
  
  close(){
    console.log("-----close--")
    this.ws.close()
  }

  send(msg){
    console.log("----send message client:" + this.props.clientName +"---msg: " + msg)
    this.ws.send(this.props.clientName + " sending:" +  msg)
  }
}


class SimpleViewApp extends Component {
  constructor(props) {
    super(props);
    this.timerId = 0;
    this.count = 0;
    this.clientCount = 20;
    this.ws = [];

    for (let i=0; i<this.clientCount; i++){
      this.ws.push(new wsClient({uri:"ws://localhost:8080", clientName: "c"+(i+1)}))
    }
    //this.ws1 = new wsClient({uri:"ws://localhost:8080", clientName: "c1"});
    //this.ws2 = new wsClient({uri:"ws://localhost:8080", clientName: "c2"});
    //this.ws3 = new wsClient({uri:"ws://localhost:8080", clientName: "c3"});
    //console.log("-----before close----")
    //ws1.close()
    //console.log("----after close----")
  }

  sendRandom = () => {

  }

  componentDidMount = () => {
    console.log("component did mount")
    this.timerId = setInterval(()=>{
      console.log("in setTimeout ----");
      //return;
      this.count++;
      console.log("################################# sending msg Interation count:" + this.count);

      for(let i=0; i< this.clientCount; i++){
        this.ws[i].send(makeid(200))
      }
      // this.ws1.send(makeid(200));
      // this.ws2.send(makeid(200));
      // this.ws3.send(makeid(200));
      if(this.count == 10){
        console.log("----------cancelling timer interval---------------")
        clearInterval(this.timerId);
        this.timerId = 0;
        setTimeout(()=>{
          console.log("############################closing all connections....")
        for(let i=0; i< this.clientCount; i++){
          this.ws[i].close()
        }
        // this.ws1.close();
        // this.ws2.close();
        // this.ws3.close();
         }, 5000)
      }
    }, 5000);


  };

  render() {
    return (
      <View style={{margin:100, fontSize: 32, backgroundColor:'yellow'}}>
        <Text>HI</Text>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  baseText: {
    fontFamily: "Cochin"
  },
  titleText: {
    fontSize: 20,
    fontWeight: "bold"
  }
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);


