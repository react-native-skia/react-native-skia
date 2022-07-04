'use strict';

const React = require('react');

const {
  StyleSheet,
  View,
  Text,
  TextInput,
  AppRegistry
} = require('react-native');


const SimpleViewApp = React.Node = () => {

   let defaultReturnKey='none';
   let defaultKBType='numeric';
   let defaultKBTheme='light';
   let needOSK=true;
   let autoEnableReturnKey=true;
   let value;

   const [KeyBoardType, onKeyBoardType] = React.useState(defaultKBType);
   const [KeyBoardAppearence, onKeyBoardAppearence] = React.useState(defaultKBTheme);
   const [returnKeyType, onreturnKeyType] = React.useState(defaultReturnKey);
   const [showSoftInputOnFocus, onshowSoftInputOnFocus] = React.useState(needOSK);
   const [enablesReturnKeyAutomatically, onenablesReturnKeyAutomatically] = React.useState(autoEnableReturnKey);

    const TextInputOSKProps = () => {
      let launchConfig = "KBTYPE:" + KeyBoardType + " KBTheme : "+KeyBoardAppearence+" ReturnKEyType: "+returnKeyType+" enablesReturnKeyAutomatically : " + enablesReturnKeyAutomatically;
      return(
          <View style={styles.Outtercontainer}>
          <Text style={styles.baseText}>OSK Properties</Text>
          <View style={styles.container}>
          <Text style={styles.text}>KeyBoardAppearence: 1. numeric 2. url 3. default</Text>
          <TextInput showSoftInputOnFocus={false}
            onSubmitEditing={(e) => {
             value = (e.nativeEvent.text != "")  ? e.nativeEvent.text : defaultKBType
             onKeyBoardType(value)}}
            style={styles.TextInput}/>
          </View>
          <View style={styles.container}>
          <Text style={styles.text}>KeyBoardAppearence: 1. light 2.dark 3. default</Text>
          <TextInput  showSoftInputOnFocus={false}
            onSubmitEditing={(e) => {
              value = (e.nativeEvent.text != "")  ? e.nativeEvent.text : defaultKBTheme
              onKeyBoardAppearence(value)}}
            style={styles.TextInput}/>
          </View>
          <View style={styles.container}>
          <Text style={styles.text}>returnKeyType: 1. search 2. done 3. none</Text>
          <TextInput showSoftInputOnFocus={false}
            onSubmitEditing={(e) => {
              value = (e.nativeEvent.text != "") ? e.nativeEvent.text : defaultReturnKey
              onreturnKeyType(value)}}
            style={styles.TextInput}/>
          </View>
          <View style={styles.container}>
          <Text style={styles.text}>showSoftInputOnFocus: 1. true 2. false </Text>
          <TextInput  showSoftInputOnFocus={false}
             onSubmitEditing={(e) => {
               value=needOSK;
               if(e.nativeEvent.text == "false") value=false;
               if(e.nativeEvent.text == "true") value=true;
               onshowSoftInputOnFocus(value)}}
            style={styles.TextInput}/>
          </View>
          <View style={styles.container}>
          <Text style={styles.text}>enablesReturnKeyAutomatically: 1. true 2. false</Text>
          <TextInput  showSoftInputOnFocus={false}
            onSubmitEditing={(e) => {
              value=autoEnableReturnKey;
              if(e.nativeEvent.text == "false") value=false;
              if(e.nativeEvent.text == "true") value=true;
               onenablesReturnKeyAutomatically(value)}}
            style={styles.TextInput}/>
          </View>
          <TextInput 
            value="Launch OSK"
            placeholder={launchConfig}
            keyboardType={KeyBoardType}
            enablesReturnKeyAutomatically={enablesReturnKeyAutomatically}
            returnKeyType={returnKeyType}
            showSoftInputOnFocus={showSoftInputOnFocus}
            keyboardAppearance={KeyBoardAppearence}
            textAlign='center'
            onSubmitEditing={console.log("KeyBoardType", KeyBoardType ,"enablesReturnKeyAutomatically", enablesReturnKeyAutomatically,"returnKeyType", returnKeyType,"showSoftInputOnFocus",showSoftInputOnFocus,"KeyBoardAppearence",KeyBoardAppearence,"launchConfig",launchConfig )}
            style={styles.input}
            />
          </View>
      )
    }

  return (
    TextInputOSKProps()
  );
}

const styles = StyleSheet.create({
  TextInput:{
    margin:10,
    backgroundColor:'#61dafb',
    width:150,
    height:50,
    textAlign: "center",
  },
  container: {
    flex: 1,
    flexDirection: "row",
    justifyContent: "space-between",
    backgroundColor: '#eaeaea',
    padding: 10,
    margin: 10,
    width:700,
    height:200,
    borderRadius: 6,
    borderWidth: 4,
  },
  Outtercontainer: {
    flex: 1,
    justifyContent: "space-between",
    backgroundColor: "darkseagreen",
    padding: 20,
    margin: 10,
    width:800,
    height:600,
    borderRadius: 6,
    borderWidth: 4,
    alignItems:"center"
  },

  input: {
  margin: 10,
  borderWidth: 4,
  width:150,
  height:50,
  backgroundColor: '#eaeaea',
  },
  text:{ 
    fontSize: 15,
  },
  baseText: {
    fontFamily: "Cochin",
    fontSize: 30,
    ontWeight: "bold",
  }
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
