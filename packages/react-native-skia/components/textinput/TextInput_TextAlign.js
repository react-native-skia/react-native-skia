import React,{Component} from 'react';
import { Text, TextInput, View, StyleSheet, AppRegistry } from 'react-native';

class  SimpleViewApp extends Component {
constructor(props){
  super(props);
  this.state={
    str1: "This is a state"
  }
}
render(){
  return (
    <View style={styles.container}>
      <TextInput style={styles.textBox1}
        onKeyPress={(e)=>{
          console.log("onkeypress------- = " + e.nativeEvent.key);
        }}
        placeholder="This is placeholder Text"
      >
      </TextInput>
      <Text style={{textAlign:"center",height:100, fontSize:30}}>TextInput Box 1</Text>
      
      <TextInput style={styles.textBox2}
        onKeyPress={(e)=>{
          console.log("onkeypress------- = " + e.nativeEvent.key);
        }}
        placeholder="This is placeholder Text with green color"
        placeholderTextColor="green"
        defaultValue="This is default Value"
      >
      </TextInput>
      <Text style={{textAlign:"right",height:100, fontSize:30}}>TextInput Box 2</Text>

      <TextInput style={styles.textBox3}
        onKeyPress={(e)=>{
          console.log("onkeypress------- = " + e.nativeEvent.key);
        }}
        onChangeText={(string)=>{
          this.setState({str1: (string)});
        }}
        placeholderTextColor="green"
        defaultValue="This is default Value"
        value={this.state.str1}
      >
      </TextInput>
      <Text style={{textAlign:"left", height:100, fontSize:30}}>TextInput Box 3</Text>
    </View>
  );
}
}

const styles = StyleSheet.create({
  container:{
    flex: 1,
    backgroundColor: 'lightyellow',
  },
  textBox1:{
    height :80,
    fontSize: 40,
    borderWidth: 10,
    borderColor: "red",
    backgroundColor:'lightcyan',
    color:"blue",
    textAlign:"center",
  },
  textBox2:{
    height :80,
    fontSize: 30,
    borderWidth: 10,
    borderColor: "red",
    backgroundColor:'lightcyan',
    color:"blue",
    textAlign:"right",
  },
  textBox3:{
    height :80,
    fontWeight:"bold",
    fontSize: 50,
    borderWidth: 10,
    borderColor: "red",
    backgroundColor:'lightcyan',
    color:"blue",
    textAlign:"left",
  },
});

//export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

