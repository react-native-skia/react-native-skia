import  React, {Component} from 'react';
import { Text, AppRegistry,View, StyleSheet, TouchableHighlight, TouchableOpacity, TextInput, Pressable } from 'react-native';


export default class App extends Component{

  constructor(props){
    super(props);
    this.state = {
      fetchOrXhr: "xhr",
      URI: "http://localhost:3000/posts",
      id: "",
      name: "",
      responseText: ""
    }
  }

  xhrHandler = (method) => {
    console.log("in xhrHandler------------: " + method)
    switch(method){
      case "GET":
         var xhrGet = new XMLHttpRequest();
         xhrGet.withCredentials = true;
         xhrGet.addEventListener("readystatechange", function() {
         if(this.readyState === 4) {
           console.log(this.responseText);
           this_.setState({responseText: this.responseText})
         }
         });
         xhrGet.open(method, this.state.URI);
         xhrGet.send();
         break;
      case "PUT":
         const xhrPut = new XMLHttpRequest();
         xhrPut.open('PUT',this.state.URI+'/'+this.state.id, true); 
         xhrPut.setRequestHeader("Accept", "application/json");
         xhrPut.setRequestHeader("Content-Type", "application/json");
         xhrPut.onreadystatechange = function () {
         if (xhrPut.readyState === 4) {
           console.log(xhrPut.status);
           console.log(xhrPut.responseText);
           this_.setState({responseText: this.responseText})
          }};
          var data={};
          data.title = this.state.name;
          var json = JSON.stringify(data);
          xhrPut.send(json);
          break;
      case "PATCH":
         const xhrPatch = new XMLHttpRequest();
         xhrPatch.open('PATCH',this.state.URI+'/'+this.state.id, true); 
         xhrPatch.setRequestHeader("Accept", "application/json");
         xhrPatch.setRequestHeader("Content-Type", "application/json");
         xhrPatch.onreadystatechange = function () {
         if (xhrPatch.readyState === 4) {
           console.log(xhrPatch.status);
           console.log(xhrPatch.responseText);
           this_.setState({responseText: this.responseText})
         }};
         var data={};
         data.title = this.state.name;
         var json = JSON.stringify(data);
         xhrPatch.send(json);
         break;
      case "DELETE":
         const xhrDelete = new XMLHttpRequest();
         xhrDelete.open('DELETE',this.state.URI+'/'+this.state.id, true); 
         xhrDelete.setRequestHeader("Accept", "application/json");
         xhrDelete.setRequestHeader("Content-Type", "application/json");
         xhrDelete.onreadystatechange = function () {
         if (xhrDelete.readyState === 4) {
           console.log(xhrDelete.status);
           console.log(xhrDelete.responseText);
         }};
         xhrDelete.send();
         break;
      case "POST":
         const xhrPost = new XMLHttpRequest();
         xhrPost.open('POST',this.state.URI, true); 
         xhrPost.setRequestHeader("Accept", "application/json");
         xhrPost.setRequestHeader("Content-Type", "application/json");
         xhrPost.onreadystatechange = function () {
         if (xhrPost.readyState === 4) {
           console.log(xhrPost.status);
           console.log(xhrPost.responseText);
         }};
         var data={};
         data.title = this.state.name;
         var json = JSON.stringify(data);
         xhrPost.send(json);
         break;
      default:
         break;
    }
  }

  fetchHandler = (method) => {
    console.log("in fetchhandle-----------: " + method)
    switch(method){
       case "GET":
          var requestOptions = {
              method: 'GET',
              redirect: 'follow'
            };
          fetch(this.state.URI, requestOptions)
          .then(response => response.text())
          .then(result => {console.log(result); this.setState({responseText: result})})
          .catch(error => console.log('error', error));
          break;
       case "PUT":
          fetch(this.state.URI+'/'+this.state.id, {
            method: 'PUT',
            body: JSON.stringify({
              id: this.state.id,
              title: this.state.name,
              userId: 4,
            }),
            headers: { 'Content-type': 'application/json; charset=UTF-8',},
          })
          .then(response => response.text())
          .then(result => {console.log(result); this.setState({responseText: result})})
          .catch(error => console.log('error', error));
          break;
       case "PATCH":
          fetch(this.state.URI+'/'+this.state.id, {
            method: 'PATCH',
            body: JSON.stringify({
              title:this.state.name,
            }),
            headers: {'Content-type': 'application/json; charset=UTF-8',},})
          .then(response => response.text())
          .then(result => {console.log(result); this.setState({responseText: result})})
          .catch(error => console.log('error', error));
          break;
       case "DELETE":
          fetch(this.state.URI+'/'+this.state.id, {method: 'DELETE'})
          .then(response => response.text())
          .then(result => {console.log(result); this.setState({responseText: result})})
          .catch(error => console.log('error', error));
          break;
       case "POST":
          fetch(this.state.URI, {
            method: 'POST',
            body: JSON.stringify({
              title: this.state.name,
            }),
            headers: {'Content-type': 'application/json; charset=UTF-8',},})
          .then((response) => response.json())
          .then((json) => console.log(json));
          break;
       default:
          break;
    }
  }

  onPress = (method) => {
    this_ = this;
    if(this.state.fetchOrXhr == "xhr"){
      this.xhrHandler(method);
    } else{
      this.fetchHandler(method)
    }
  }

  render(){

  return (
    <View style={styles.container}>
      <Text>XHR OR FETCH:</Text>
      <TextInput showSoftInputOnFocus={true} style={styles.input} placeholder={"CHOOSE:fetch or xhr"} value={this.state.fetchOrXhr} 
                 onChangeText={(x)=>{this.setState({fetchOrXhr: x})}}> </TextInput>
      <TextInput showSoftInputOnFocus={false} style={styles.input} placeholder={"URI"} value={this.state.URI} onChangeText={(x)=>{this.setState({URI: x})}} ></TextInput>
      <View style = {{backgroundColor: "lightblue"}}>
        <Text style={styles.paragraph}>ITEM To be added</Text>
        <Text>ID</Text>
        <TextInput showSoftInputOnFocus={false} style={styles.input} placeholder={"ID:"} value={this.state.id} onChangeText={(x)=>{console.log("---id:" + x); this.setState({id: x})}}  ></TextInput>
        <Text>NAME</Text>
        <TextInput showSoftInputOnFocus={false} style={styles.input} placeholder={"Name:"} value={this.state.name}  onChangeText={(x)=>{console.log("---name:" + x); this.setState({name: x})}}></TextInput>
      </View>

      <TouchableHighlight isTVSelectable="true" activeOpacity={0.1}  underlayColor = {"orange"} style={styles.button} onPress={()=>{this.onPress("POST")}} >
        <Text>METHIOD - POST</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable="true" activeOpacity={0.1} underlayColor = {"orange"} style={styles.button} onPress={()=>{this.onPress("GET")}} >
        <Text>METHIOD - GET</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable="true" activeOpacity={0.1}  underlayColor = {"orange"} style={styles.button} onPress={()=>{this.onPress("PUT")}} >
        <Text>METHIOD - PUT</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable="true"  activeOpacity={0.1} underlayColor = {"orange"} style={styles.button} onPress={()=>{this.onPress("PATCH")}}>
        <Text>METHIOD - PATCH</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable="true"  activeOpacity={0.1} underlayColor = {"orange"} style={styles.button} onPress={()=>{this.onPress("DELETE")}}>
        <Text>METHIOD - DELETE</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable="true" activeOpacity={0.1} underlayColor = {"orange"} style={styles.button} onPress={()=>{this.setState({responseText: ""})}}>
        <Text>CLEAR-SCREEN</Text>
      </TouchableHighlight>
      <View style = {{backgroundColor: "lightblue", borderColor: "red",borderWidth: 2}}>
        <Text style={styles.paragraph}>
        {"URI RESPONSE:"}
        </Text>
        <Text style={styles.paragraph1}>
        {"---------DATA------\n"}
        {this.state.responseText}
        </Text>
      </View>
    </View>
  );

  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    backgroundColor: '#ecf0f1',
    padding: 8,
  },
  paragraph: {
    margin: 5,
    fontSize: 16,
    fontWeight: 'bold',
    textAlign: 'center',
    
  },
  paragraph1: {
    margin: 5,
    fontSize: 10,
  },
  button:{
    margin: 5,
    backgroundColor: "lightpink",
    borderWidth: 2,
    width: 200,
    height: 25,
    textAlign: "center",
    justifyContent: "center",
    shadowColor: "lightblue",
    shadowRadius: 5,
    shadowOffset: {width: 3, height: 3},
  },
  input: {
    height: 25,
    width: "70%",
    backgroundColor:"pink",
    margin: 5,
    borderWidth: 1,
    padding: 10,
  },
});
AppRegistry.registerComponent('SimpleViewApp', () => App);