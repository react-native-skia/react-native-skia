import React, { useState,useRef} from "react";
import { TextInput,View,ImageBackground,TouchableOpacity,Text,Animated,Image} from "react-native";
import { Dimensions , StyleSheet} from "react-native";

import SampleVODPage from './SampleVODPage';

const windowSize = Dimensions.get('window');

const TextInputBlock = (props) => {
    let [tiState,setTiState] = useState({borderColor:'black',shadowOpacity:0});
    const onFocus = () => {
       setTiState({borderColor:'lightblue',shadowOpacity:1});
    }
    const onBlur = () => {
       setTiState({borderColor:'black',shadowOpacity:0});
    }
    return (
       <View style={[styles.textinputView,{borderColor:tiState.borderColor,shadowOpacity:tiState.shadowOpacity}]}>
          <TextInput style={styles.textinput} placeholder={props.placeholderText} placeholderTextColor="darkgrey"
             secureTextEntry={props.secured} defaultValue={props.defaultValue}
             onFocus={onFocus} onBlur={onBlur} onChangeText={props.onChangeText}/>
       </View>
   );

}

const SampleLoginPage = () => {

    let [loginStatus, setLoginStatus] = useState(false);
    let [requestStatus,setRequestStatus] = useState({text:"Load",viewColor:"darkcyan"});
    let [content,setContent] = useState([]);
    let [spinValue] = useState(new Animated.Value(0));
    let [serverIp,setServerIp] = useState("http://192.168.0.105:9081");

    const startFetchVODData = () => {
       var request = new XMLHttpRequest();
       request.open('GET',serverIp+'/dataContent.json');
       request.responseType = "json"
       request.timeout = 10
       request.onreadystatechange = (e) => {
          console.log("request state change:" + request.status);
          if(request.status == 200) {
            setContent(request.response);
            setRequestStatus({text:"WE ARE READY TO GO !! ",viewColor:"green"})
            setTimeout(()=> {
               setLoginStatus(true);
            },2000);
          } else {
            console.error("request error:",request.status);
            setRequestStatus({text:"Error !!",viewColor:"red"})
          }
       }
       request.onerror = (e) => {
          console.error("request error:",request.status);
          setRequestStatus({text:"Error !!",viewColor:"red"})
       }
       request.ontimeout = (e) => {
          console.error("request timeout:",request.status);
          setRequestStatus({text:"Error !!",viewColor:"red"})
       }
       console.log("request send");
       request.send();
    }

    const onPress = () => {
       setRequestStatus({text:"Loading",viewColor:"royalblue"})
       setTimeout(()=> {
         startFetchVODData()
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
              <Animated.View style={[animatedStyle]}>
                     <Image source={require('./images/loadingIcon.png')} style={{width:125,height:125}} resizeMode='cover'/>
              </Animated.View>
          );

       } else {
          return (
              <TouchableOpacity style={[styles.submitView,{backgroundColor:requestStatus.viewColor,borderWidth:5}]} onPress={onPress} activeOpacity={0.85}>
                  <Text style={{margin:5,color:'black' , fontSize:windowSize.height/35 , fontWeight:'bold',textAlign:'center'}}>{requestStatus.text}</Text>
              </TouchableOpacity>
          );
       }
    }

    const loginPage = () => {
      return (
          <ImageBackground style={styles.backgroundimage} source={require('./images/bg.jpg')} resizeMode='cover'>
             <Image style={{ margin:50,width: 200, height: 200 }} source={require('react-native/Libraries/NewAppScreen/components/logo.png')}/>
             <TextInputBlock placeholderText={"Username"} secured={false} defaultValue={""} ></TextInputBlock>
             <TextInputBlock placeholderText={"Password"} secured={true}  defaultValue={""} ></TextInputBlock>
             <Text style={{fontSize:windowSize.height/35, color:'darkgrey', textDecorationLine:'underline', textDecorationColor:'darkgrey'}}>{"Server IP"}</Text>
             <TextInputBlock placeholderText={"Server IP"} secured={false} defaultValue={serverIp} onChangeText={setServerIp}></TextInputBlock>
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
       width : '40%',
       height : windowSize.height/20,
       borderWidth : 5,
       backgroundColor:'dimgrey',
       shadowColor : 'black',
       shadowRadius : 10,
       shadowOffset : {width:5, height:30},
    },
    textinput: {
       color :'white',
       fontSize:windowSize.height/35,
    },
    submitView: {
       alignItems : 'center',
       padding : 20,
       color :'white',
    }
});

export default SampleLoginPage

