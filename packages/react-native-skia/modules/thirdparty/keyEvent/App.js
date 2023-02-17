import React, { useState, useEffect } from 'react';
import { View, AppRegistry, Image, Text,TextInput, TVEventHandler, TouchableOpacity, StyleSheet,ScrollView,Pressable} from 'react-native';
import KeyEvent from 'react-native-keyevent';
console.log('TVEventHandler: ', TVEventHandler);


const FocusableComponent = (props) =>  {
   let [state, setState] = useState({imgscale: 1,bw:0});
   const onPress = () => {}

   const onBlur = (e) => {
       console.log("onBlur---------" )
       setState({ imgscale: 1, bw:0});
   }

   const onFocus = (e) => {
       console.log("onFocus---------" )
       setState({ imgscale: 1.2 , bw:2});
   }

   
  return (
    <Pressable isTVSelectable='true' onBlur={onBlur} onFocus={onFocus} style={styles.elementView} >
    <Image style={{transform:[{scale: state.imgscale}],width:300,height:160,marginLeft:30,marginTop:16}} 
     source={{url:"https://reactnative.dev/img/tiny_logo.png"}}>
    </Image>
    </Pressable>
    );
}


// let textkey;
// let tvKey;
class SimpleViewApp extends React.Component {
  constructor(props) {
    super(props);

    this._tvEventHandler = null;
    this.state = {
      bgColor: '#FFFFFF',
      tiBgColor:"black",
      textkeyDown:"",
      textkeyUp:"",
      textkeyMultiple:0,
    }

  }
  _reactnativeKeyevent(){
    // if you want to react to keyDown
    KeyEvent.onKeyDownListener((keyEvent) => {
      console.log(`onKeyDown keyCode: ${keyEvent.keyCode}`);
      console.log(`Action: ${keyEvent.action}`);
      console.log(`Key: ${keyEvent.pressedKey}`);
      this.setState({textkeyDown: keyEvent.pressedKey});
      this.setState({textkeyMultiple:0});
    });

    // if you want to react to keyUp
    KeyEvent.onKeyUpListener((keyEvent) => {
      console.log(`onKeyUp keyCode: ${keyEvent.keyCode}`);
      console.log(`Action: ${keyEvent.action}`);
      console.log(`Key: ${keyEvent.pressedKey}`);
      this.setState({textkeyUp:keyEvent.pressedKey});
    });

    // if you want to react to keyMultiple
    KeyEvent.onKeyMultipleListener((keyEvent) => {
      console.log(`onKeyMultiple keyCode: ${keyEvent.keyCode}`);
      console.log(`Action: ${keyEvent.action}`);
      console.log(`Characters: ${keyEvent.characters}`);
      console.log(`Repeat count : ${keyEvent.repeatcount}`);
      this.setState({textkeyMultiple:keyEvent.repeatcount});
    });
  }
  _enableTVEventHandler() {
    let that = this;
    this._tvEventHandler = new TVEventHandler();
    console.log('APP: TVEventHandler ',this._tvEventHandler);
    this._tvEventHandler.enable(this, function(cmp, evt) {
      console.log('APP: TV Key event received: ', evt);
      if(evt.eventType === "blur" || evt.eventType === "focus"){
        console.log('APP: TV Key event received: ', evt.eventType);      
      }else{
        that.setState({tvKey : evt.eventType});
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
    this._reactnativeKeyevent();
  }

  componentWillUnmount() {
    this._disableTVEventHandler();
        // if you are listening to keyDown
    KeyEvent.removeKeyDownListener();

     // if you are listening to keyUp
    KeyEvent.removeKeyUpListener();

     // if you are listening to keyMultiple
    KeyEvent.removeKeyMultipleListener();
  }

  addItems(){
    var n =4;
    var arr = [];
    for (var i=0; i<n; i++){
      arr.push(<FocusableComponent count={i}></FocusableComponent>);
    }
    return arr;
  }
  
  render() {
    console.log('bgColor: ', this.state.bgColor);
    return (
    <View 
      style={{ flex: 1,
               justifyContent: 'center',
               alignItems: 'center',
               marginTop: 8,
               backgroundColor: 'aquamarine'}}
      onLayout={() => console.log('onLayout')}>
        
        <View style={{fontSize:20,flexDirection:"row",marginTop: 8, borderColor:"red", borderWidth:2,width: 1200} }>
          <Text style={{fontSize:20, width: 550}}>
            react native key event  onKeyDown  {this.state.textkeyDown}
          </Text>
          <Text style={{fontSize:20, width: 450}}>
            onKeyUp {this.state.textkeyUp}
          </Text>
          <Text style={{fontSize:20, width: 350}}>
            onKeyMultiple {this.state.textkeyMultiple}
          </Text>
        </View>
        
      <Text style={{fontSize:20, width:550}}>
        TVevent handler event {this.state.tvKey}
      </Text>
      
      <TextInput
        style={{borderColor:this.state.tiBgColor,
                height: 50,
                width:200,
                margin: 5,
                borderWidth: 5,
                padding: 5  ,
                }}
        onFocus={()=>{ this.setState({tiBgColor: '#00FF00'})}}
        onBlur={()=>{this.setState({tiBgColor: '#000000'})}} 
      />
      
      <View style={{flexDirection:"row",marginTop: 8,alignContent:"space-around"} }> 
        <TouchableOpacity 
          onPress={()=>{KeyEvent.removeKeyDownListener();}} 
          style={styles.appButtonContainer}>
          <Text style={styles.appButtonText}> removeKeyDownListener</Text>
        </TouchableOpacity>
        
        <TouchableOpacity 
          onPress={()=>{KeyEvent.removeKeyUpListener();}} 
          style={styles.appButtonContainer}>
          <Text style={styles.appButtonText}> removeKeyUpListener</Text>
        </TouchableOpacity>
        <TouchableOpacity 
          onPress={()=>{KeyEvent.removeKeyMultipleListener();}} 
          style={styles.appButtonContainer}>
          <Text style={styles.appButtonText}> removeKeyMultipleListener</Text>
        </TouchableOpacity>
      </View>
      
      <View style={{flexDirection:"row",marginTop: 8,alignContent:"space-around"} }>
        <TouchableOpacity 
           onPress={()=>{KeyEvent.onKeyDownListener((keyEvent) => {
             console.log(`onKeyDown keyCode: ${keyEvent.keyCode}`);
             console.log(`Action: ${keyEvent.action}`);
             console.log(`Key: ${keyEvent.pressedKey}`);
             this.setState({textkeyDown: keyEvent.pressedKey});
            });}}
            style={styles.appButtonContainer}>
            <Text style={styles.appButtonText}> addKeyDownListener</Text>
        </TouchableOpacity>
        
        <TouchableOpacity 
          onPress={()=>{KeyEvent.onKeyUpListener((keyEvent) => {
          console.log(`onKeyDown keyCode: ${keyEvent.keyCode}`);
          console.log(`Action: ${keyEvent.action}`);
          console.log(`Key: ${keyEvent.pressedKey}`);
          this.setState({textkeyUp: keyEvent.pressedKey});
          });}} 
          style={styles.appButtonContainer}>
          <Text style={styles.appButtonText}> addKeyUpListener</Text>
        </TouchableOpacity>
        <TouchableOpacity 
          onPress={()=>{KeyEvent.onKeyMultipleListener((keyEvent) => {
          console.log(`onKeyDown keyCode: ${keyEvent.keyCode}`);
          console.log(`Action: ${keyEvent.action}`);
          console.log(`Key: ${keyEvent.pressedKey}`);
          this.setState({textkeyMultiple: keyEvent.repeatcount});
          });}} 
          style={styles.appButtonContainer}>
          <Text style={styles.appButtonText}> addKeyMultipleListener</Text>
        </TouchableOpacity>
      </View>
      
      <ScrollView style={styles.horizontalScrollView} horizontal={true}>
        {this.addItems()}
      </ScrollView>
    
    </View>
    );
  }
}

const styles = StyleSheet.create({

  horizontalScrollView: {
       margin : 5,
    },
    appButtonContainer: {
    elevation: 8,
    backgroundColor: "#009688",
    borderRadius: 10,
    paddingVertical: 10,
    paddingHorizontal: 12
  },
  appButtonText: {
    fontSize: 18,
    color: "#fff",
    fontWeight: "bold",
    alignSelf: "center",
    textTransform: "uppercase"
  },
elementView: {
       width : 360, // Thumbnail image size 
       height : 192,
       shadowColor: 'black',
    }
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);