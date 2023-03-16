import * as React from "react";
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  Platform,
  TouchableHighlight
} from "react-native";

const SimpleViewApp = () => {
  const [responseText, setResponseText] = React.useState();
  const GetSystemName = () => {
    let responseString =
    "Platform OS:" +Platform.OS+"\n" +
    "Platform Os Version():" +Platform.Version +"\n" +
    "Platform IsTv():" +Platform.isTV +"\n" +
    "Platform IsPad():" +Platform.isPad +"\n" +
    "Platform Testing():" +Platform.isTesting +"\n" +
    "Platform Constants():" +Platform.constants + "\n"+
    "Platform Constants System Name:" +Platform.constants.systemName +"\n" +
    "Platform Constant forceTouchAvailable:"+Platform.constants.forceTouchAvailable +"\n" +
    "Platform Constant reactNativeVersion:" +Platform.constants.reactNativeVersion.major+"." 
                                            +Platform.constants.reactNativeVersion.minor+"." 
                                            +Platform.constants.reactNativeVersion.patch+"-"
                                            +Platform.constants.reactNativeVersion.prerelease+ "\n"+
    "Platform Constant interfaceIdiom:" +Platform.constants.interfaceIdiom +"\n" +
    "Platform Constant Os Version:" +Platform.constants.osVersion +"\n" +
    "Platform Constant is Testing:" +Platform.constants.isTesting +"\n" +
    "Platform Select:" +Platform.select +"\n" 
   
      setResponseText(responseString);
    
  }; 

  return (
    <View style={styles.mainView}>
      <View>
        <TouchableHighlight
          isTVSelectable="true"
          underlayColor="#FFBA08"
          style={styles.controlItems}
          onPress={GetSystemName}>
          <Text style={{ margin: 2, fontSize: 19,  }}>{"React-Native-Platform"}</Text>
        </TouchableHighlight>
      </View>
      <View>
        <Text style={{ marginVertical: 20, marginLeft: 20, fontSize: 18 }}>
          {responseText}
        </Text>
      </View>
    </View>
  );
};
const styles = StyleSheet.create({
  mainView: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    ...Platform.select({
        android:{
                  backgroundColor: 'green',
                },
        ios:{
              backgroundColor: 'lightgrey',
            },
        default:{
                  // other platforms, web for example
                  backgroundColor: 'blue',
                },
    }),
  },
  controlItems: {
    margin: 18,
    alignItems: "center",
    justifyContent: "center",
    width: 280,
    height: 90,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  }
});
export default SimpleViewApp;
AppRegistry.registerComponent("SimpleViewApp", () => SimpleViewApp);