import React, {Component} from 'react';
import { Text, View, StyleSheet, Keyboard, TextInput, TouchableOpacity,AppRegistry } from 'react-native';


export default class SimpleViewApp extends Component{
  constructor(props){
    super(props)
    this.list = [];
    let i;
    let j;
    let k;
    //let a = 0;
  }

  showCallback = (x) => {
    console.log("------------showCallback:", x);
  }


  componentDidMount(){

    let a=0;
    console.log("--------------componentDidMount--------------------")
    this.i = Keyboard.addListener("keyboardDidShow", ()=>{this.showCallback(1)})
    this.a++
    this.j = Keyboard.addListener("keyboardDidShow", ()=>{this.showCallback(2)})
    a++
    this.k = Keyboard.addListener("keyboardDidShow", ()=>{this.showCallback(3)})
    Keyboard.addListener("keyboardWillShow", ()=>{this.showCallback(3)})
    Keyboard.addListener("keyboardWillHide", ()=>{this.showCallback(3)})
    Keyboard.addListener("keyboardDidHide", ()=>{this.showCallback(3)})


    this.list.push(this.i)
    this.list.push(this.j)
    this.list.push(this.k)

    for(;a<10;a++){
      console.log("------------ a:",this.a );
      l = Keyboard.addListener("keyboardDidShow", this.showCallback);
      this.list.push(l);
    }

    console.log("------------this.list.len:" + this.list.length)

  }

  onPress = () => {
    console.log("-----onPress this.list.len:" + this.list.length)
    //Keyboard.removeAllListeners("keyboardDidShow")
    this.i.remove();
    //for (index = 0; index < this.list.length; index++) {
    //console.log("iterating the for loop ",this.list[index].remove());
    //}
    //Keyboard.removeListener("keyboardDidShow", ()=>{this.showCallback(1)})
    //Keyboard.removeListener("keyboardDidShow", this.showCallback)
    // this.list[this.list.length-1].remove()
    // this.list.pop()

  }


  onPressRemoveAll = () => {
    console.log("-----onPress this.list.len:" + this.list.length)
    Keyboard.removeAllListeners("keyboardDidShow")
    //Keyboard.removeListener("keyboardDidShow", ()=>{this.showCallback})
  }

  render(){
    return(
      <View style={styles.container}>
        <Text style={styles.paragraph}>keyboard</Text>
        <TouchableOpacity style={styles.button} onPress={this.onPress}>
            <Text>Revome a Listener</Text>
        </TouchableOpacity>

        <TouchableOpacity style={styles.button} onPress={this.onPressRemoveAll}>
            <Text>RemoveAll</Text>
        </TouchableOpacity>
        <TextInput style={styles.ti} onSubmitEditing={Keyboard.dismiss()}/>
      </View>
    )
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'burlywood',
    padding: 8,
  },
  paragraph: {
    margin: 24,
    fontSize: 18,
    fontWeight: 'bold',
    textAlign: 'center',
  },

  ti: {
    margin: 20,
    fontSize: 18,
    fontWeight: 'bold',
    width: 200,
    height: 100,
    borderWidth: 3,
    borderColor: "black",
    padding: 10
  },

  th: {
    width: 200,
    height: 50,
    borderWidth: 3,
    borderColor: 'yellow',
    margin: 10,
    padding: 10,
    marginTop: '8%',
    marginBottom: '8%'
  },
    button: {
    alignItems: "center",
    backgroundColor: "blue",
    padding: 10
  },
});


//export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
