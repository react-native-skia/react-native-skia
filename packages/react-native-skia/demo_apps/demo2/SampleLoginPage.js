import React, { useState,useRef} from "react";
import { TextInput,View,ImageBackground,TouchableHighlight,Text,Animated,Image} from "react-native";
import { Dimensions , StyleSheet} from "react-native";

import SampleVODPage from './SampleVODPage';

const windowSize = Dimensions.get('window');
const localContentData = require('./dataContent.json');
let fetchServerData = true;
let serverIp = "192.168.0.7:";
let serverPort = "9081";

const TextInputBlock = (props) => {
    let [tiState,setTiState] = useState({borderColor:'black',shadowOpacity:0});
    const onFocus = () => {
       setTiState({borderColor:'#61dafb',shadowOpacity:1});
    }
    const onBlur = () => {
       setTiState({borderColor:'black',shadowOpacity:0});
    }
    const onSubmitEditing = () => {
       if(props.nextRef) {
          props.nextRef.current.focus();
       }
    }
    return (
        <TextInput ref={props.currentRef} style={[styles.textinput,styles.textinputView,{borderColor:tiState.borderColor,shadowOpacity:tiState.shadowOpacity}]}
             placeholder={props.placeholderText} placeholderTextColor="dimgrey"
             secureTextEntry={props.secured} defaultValue={props.defaultValue}
             onFocus={onFocus} onBlur={onBlur}
             onChangeText={props.onChangeText} onSubmitEditing={onSubmitEditing}/>
    );

}

const SampleLoginPage = () => {

    let [loginStatus, setLoginStatus] = useState(false);
    let [requestStatus,setRequestStatus] = useState({text:"SIGN IN"});
    let [content,setContent] = useState([]);
    let [spinValue] = useState(new Animated.Value(0));
    let usrRef = useRef();
    let pswdRef = useRef();

    const startFetchVODData = () => {
       var request = new XMLHttpRequest();
       request.open('GET',serverIp+serverPort+'/dataContent.json');
       request.responseType = "json"
       request.timeout = 10
       request.onreadystatechange = (e) => {
          console.log("request state change:" + request.status);
          if(request.status == 200) {
            setContent(request.response);
            setTimeout(()=> {
               setLoginStatus(true);
            },2000);
          } else {
            console.error("request error:",request.status);
            setRequestStatus({text:"Error"})
          }
       }
       request.onerror = (e) => {
          console.error("request error:",request.status);
          setRequestStatus({text:"Error"})
       }
       request.ontimeout = (e) => {
          console.error("request timeout:",request.status);
          setRequestStatus({text:"Error"})
       }
       console.log("request send");
       request.send();
    }

    const onPress = () => {
       setRequestStatus({text:"Loading"})
       setTimeout(()=> {
         if(fetchServerData) {
            startFetchVODData()
         } else {
            setContent(localContentData);
            setTimeout(()=> {
               setLoginStatus(true);
            },2000);
         }
       },3000);
       startRotation();
    }

    startRotation=()=>{
      console.log("start rotation animation")
      Animated.loop(
         Animated.timing(
         spinValue,
         {
            toValue: 1,
            duration: 6000,
         })
         ,{iterations:1}).start();
    }

    const spin = spinValue.interpolate({
       inputRange: [0, 1],
       outputRange: ['0deg', '2000deg']
    })

    const animatedStyle = {
       transform : [
        {
          rotate: spin
        }
      ]
    }

    const loadingButton = () => {
       if( requestStatus.text == "Loading" ) {
          return (
              <Animated.View style={[animatedStyle,{marginTop:30}]}>
                     <Image source={require('./images/loadingIcon.png')} style={{width:125,height:125}} resizeMode='cover'/>
              </Animated.View>
          );

       } else  if (requestStatus.text == "Error") {
          return (
            <>
              <TouchableHighlight underlayColor='#61dafb' style={[styles.submitView]} onPress={onPress} activeOpacity={0.65}>
                  <Text style={{margin:5,color:'dimgrey' , fontSize:windowSize.height/35 , fontWeight:'bold',textAlign:'center'}}>{"SIGN IN"}</Text>
              </TouchableHighlight>
              <Text style={{margin:15,color:'red' , fontSize:windowSize.height/45 ,textAlign:'center'}}>{"Sign in request failed.Please check your server connection."}</Text>
            </>
          );
       } else {
          return (
              <TouchableHighlight underlayColor='#61dafb' style={[styles.submitView]} onPress={onPress} activeOpacity={0.75}>
                  <Text style={{margin:5,color:'dimgrey' , fontSize:windowSize.height/40 , fontWeight:'bold',textAlign:'center'}}>{"SIGN IN"}</Text>
              </TouchableHighlight>
          );
       }
    }

    const loginPage = () => {
      return (
          <ImageBackground style={styles.backgroundimage} source={require('./images/bg.jpg')} resizeMode='cover' onLayout={()=>{usrRef.current.focus()}}>
             <Image style={{ marginTop:20,marginBottom:100,width: 200, height: 200 }} source={require('react-native/Libraries/NewAppScreen/components/logo.png')}/>
             <TextInputBlock currentRef={usrRef} nextRef={pswdRef} placeholderText={"Username"} secured={false} defaultValue={""} ></TextInputBlock>
             <TextInputBlock currentRef={pswdRef} placeholderText={"Password"} secured={true}  defaultValue={""} ></TextInputBlock>
             {loadingButton()}
          </ImageBackground>
      )
    };

    if(loginStatus) {
      return <SampleVODPage contentData={content}></SampleVODPage>;
    } else {
      return (
          loginPage()
      );
    }
}

const styles = StyleSheet.create({
    backgroundimage: {
       flex: 1,
       alignItems : 'center',
       justifyContent : 'center',
       width: windowSize.width,
       height: windowSize.height,
    },
    textinputView: {
       margin : 20,
       width : '38%',
       height : windowSize.height/15,
       borderWidth : 3,
       borderRadius : 12,
       backgroundColor:'transparent',
       shadowColor : 'black',
       shadowRadius : 10,
       shadowOffset : {width:5, height:30},
    },
    textinput: {
       color :'lightgrey',
       fontSize:windowSize.height/35,
    },
    submitView: {
       marginTop : 30,
       alignItems : 'center',
       padding: 20,
       width : '13%',
       borderWidth: 3,
       borderRadius : 80,
       backgroundColor:'transparent',
    }
});

export default SampleLoginPage

