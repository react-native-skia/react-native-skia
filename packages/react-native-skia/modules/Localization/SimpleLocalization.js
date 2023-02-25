import * as React from 'react';
import { AppRegistry, Dimensions, StyleSheet, Text,TextInput, useState, View, TouchableHighlight} from 'react-native';
import LocalizedStrings from 'react-native-localization';

const windowSize = Dimensions.get('window');

let strings = new LocalizedStrings({
    "en-US":{
      settingMenu:"EN-US : Box Settings",
      boiledEgg:"Boiled egg",
      softBoiledEgg:"Soft-boiled egg",
      choice:"How to choose the egg",
      question:"I'd like {0} and {1}, or just {0}",
      bread: "BREAD-EN-US",
      butter: "BUTTER-EN-US"
    },
    "en-IN":{
      how:"EN-IN : How do you want your egg today ?",
      boiledEgg:"Boiled egg",
      softBoiledEgg:"Soft-boiled egg",
      choice:"How to choose the egg",
      question:"I'd like {0} and {1}, or just {0}",
      bread: "BREAD-EN-IN",
      butter: "BUTTER-EN-IN"
    },
    en:{
      how:"EN - How do you want your egg today ?",
      boiledEgg:"Boiled egg",
      softBoiledEgg:"Soft-boiled egg",
      choice:"How to choose the egg",
      question:"I'd like {0} and {1}, or just {0}",
      bread: "BREAD-EN",
      butter: "BUTTER-EN"
    },
    it: {
      settingMenu:"IT : XOXO Setto",
      boiledEgg:"Uovo sodo",
      softBoiledEgg:"Uovo alla coque",
      choice:"Come scegliere l'uovo",
      question:"Vorrei {0} e {1}, o solo {0}",
      bread: "PANE-IT",
      butter: "BURRO-IT"
    }
    });

const formLocaleContent = () =>  {
  let localizedTextContent = "Interface Language : " + strings.getInterfaceLanguage() + '\n' +
                    "Language : " + strings.getLanguage() + '\n' +
                    "Available Languages : " + JSON.stringify(strings.getAvailableLanguages()) + '\n' +
                    "Formatted Text : " +  strings.formatString(strings.question, strings.bread, strings.butter) + '\n' +
                    "Content : " + strings.settingMenu;
  return localizedTextContent;
}


const SimpleViewApp = React.Node = () => {

  let [tiborder, setTiBorder] = React.useState('black');
  const [inputText, setInputText] = React.useState("");

  let [loacleTextContent, setLoacleTextContent] = React.useState(formLocaleContent());

  const onTextSubmit = (e) => {
     console.log("LanguageCode:" + e.nativeEvent.text);
     setInputText(e.nativeEvent.text)
  }
  const onGetInterfaceLanguage = () => {
    localizedTextContent = "Interface Language : " + strings.getInterfaceLanguage() + '\n';
    setLoacleTextContent(localizedTextContent);
  }
  const onGetAvailableLanguages = () => {
    localizedTextContent = "Available Languages : " + JSON.stringify(strings.getAvailableLanguages()) + '\n';
    setLoacleTextContent(localizedTextContent);
  }
  const onGetLanguage = () => {
    localizedTextContent = "Language : " + strings.getLanguage() + '\n';
    setLoacleTextContent(localizedTextContent);
  }
  const onFormatString = () => {
    localizedTextContent = "Formatted Text : " +  strings.formatString(strings.question, strings.bread, strings.butter) + '\n';
    setLoacleTextContent(localizedTextContent);
  }
  const onSetLanguage = () => {
    console.log("Set Language Code : " + inputText);
    strings.setLanguage(inputText)

    localizedTextContent = formLocaleContent();
    setLoacleTextContent(localizedTextContent);
  }

  return (
    <View style={styles.mainView}>
      <Text style={styles.posterView}>
        {loacleTextContent}
      </Text>

      <View style={styles.controlBoxView}>
        <TouchableHighlight isTVSelectable='true' underlayColor='#FFBA08' style={styles.controlItems} onPress={onGetAvailableLanguages}>
          <Text style={{margin:2,fontSize:18}}>{"getAvailableLanguages"}</Text>
        </TouchableHighlight>
        <TouchableHighlight isTVSelectable='true' underlayColor='#FFBA08' style={styles.controlItems} onPress={onGetInterfaceLanguage}>
          <Text style={{margin:2,fontSize:18}}>{"getInterfaceLanguage"}</Text>
        </TouchableHighlight>
        <TouchableHighlight isTVSelectable='true' underlayColor='#FFBA08' style={styles.controlItems} onPress={onGetLanguage}>
          <Text style={{margin:2,fontSize:18}}>{"getLanguage"}</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable='true' underlayColor='#FFBA08' style={styles.controlItems} onPress={onFormatString}>
	      <Text style={{margin:2,fontSize:18}}>{"formatString"}</Text>
      </TouchableHighlight>
      <TouchableHighlight isTVSelectable='true' underlayColor='#FFBA08' style={styles.controlItems} onPress={onSetLanguage}>
	      <Text style={{margin:2,fontSize:18}}>{"setLanguage"}</Text>
      </TouchableHighlight>
      <TextInput style={[styles.controlItems,{borderColor:tiborder,fontSize:18}]}
	             placeholder={"LanguageCode"}
	             onSubmitEditing={onTextSubmit}/>
      </View>

    </View>
  );
};

const styles = StyleSheet.create({
  mainView: {
    flex:1,
    backgroundColor:'#444',
  },
  posterView:{
    position:'absolute',
    top:'70',
    left:windowSize.width/4,
    width:(windowSize.width/2)-20,
    height:(windowSize.height/2)-100,
    backgroundColor:'#FFBA08',
    borderColor:'black',
    borderWidth:2,
    fontSize:22,
  },
  controlBoxView: {
    position:'absolute',
    top:windowSize.height/2+40,
    width:windowSize.width-40,
    margin:20,
    flexDirection:'row',
    justifyContent:'center',
    backgroundColor: 'transparent',
  },
  controlItems: {
    margin:10,
    alignItems:'center',
    justifyContent:'center',
    width:220,
    height:100,
    borderColor:'black',
    borderWidth:2,
    backgroundColor:'#35393C',
  }
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
