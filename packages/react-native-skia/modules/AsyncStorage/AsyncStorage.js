/**
* Sample React Native App
* https://github.com/facebook/react-native
*
* @format
* @flow strict-local
*/
import React, {
    Component
} from 'react';
import {
    Text,
    TextInput,
    View,
    AppRegistry,
    StyleSheet,
    TouchableOpacity,
    TouchableHighlight
} from 'react-native';

import AsyncStorage from '@react-native-async-storage/async-storage';

let keys = [];
const makeid =  (length) => {
  var result = '';
  var characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  var charactersLength = characters.length;
  for (var i = 0; i < length; i++) {
      result += characters.charAt(Math.floor(Math.random() * charactersLength));
  }
  return result;
}

const storeData = async (key, value) => {
  try {
    console.log("---------------storing data: ", value)
    AsyncStorage.setItem(key, value)
  } catch (e) {
    console.error(e);
  }
}

const storeRandomData =  () => {
  try {
    console.log("---------------storing data: ")
    for(var i=0;i<5;i++){
     keys[i] = makeid(6);
     AsyncStorage.setItem(keys[i], "VALUE")
    }
  } catch (e) {
    // saving error
  }
}

const getData = async (key) => {
  AsyncStorage.getItem(key, (err, result) => {
    if (result == null) {
      console.error(`error from getData  > err.message, err.key`, err.message, err.key);
      return;
    }
    console.log(result);
  });
}

const _retrieveData = async (key) => {
  try {
    const value = await AsyncStorage.getItem(key);
    console.info("_retrieveData ", value);
  } catch (error) {
    // Error retrieving data
    console.error("_retrieveData error ", error.message, error.key);
  }
};
const _retrieve_100_Data = async () => {
  try {
    console.log("get_100_data",Date.now());
    for(let i=0;i<5;i++){
      const value = AsyncStorage.getItem(keys[i], ()=>{});
    }
    console.log("get_100_data",Date.now());
  } catch (error) {
     // Error retrieving data
     console.error("_retrieveData error ", error);
  }
};
var gKey = 0;
function getObjKey() {
    gKey++;
    return gKey;
}

removeValue = async (key) => {
  try {
    await AsyncStorage.removeItem(key)
  } catch(e) {
    console.error(e.message, e.key)
    return
  }
  console.log('Done.')
}

multiStore = async(multi_set_pairs) =>{
   try {
    await AsyncStorage.multiSet(multi_set_pairs);
  } catch(e) {
    console.log(e);
  }
}

multipleRemove = async(keys) =>{
  try {
    await AsyncStorage.multiRemove(keys)
  } catch(e) {
    console.error(e);
  }
}

getAsyncData = async (key) => {
  try {
    const data = await AsyncStorage.getItem(key);
    console.log(`----------------getAsyncData data ${key}`, data);
  } catch (e) {
    console.error(`----------------getAsyncData  ${key} error`, e.message, e.key);
  }
}

multiGetAsync = async (keys) => {
  try {
    const data = await AsyncStorage.multiGet(keys);
    console.log(`----------------getAsyncData data`, data);
  } catch (e) {
    console.error(`----------------getAsyncData error`, e);
  }
}

multiGetCB = (keys) => {
  AsyncStorage.multiGet(keys, (err, values) => {
    console.log("multiGetCB Values",values);
    console.error("Error multiGetCB", err);
  });
}

TEST_GET_ALL_KEYS = async () => {
  let keys = []
  try {
    keys = await AsyncStorage.getAllKeys()
    console.error(keys)
  } catch(e) {
    console.error("get all keys error")
    return;
  }
  console.log(keys)
}

TEST_CLEAR_DATA = async () => {
  try {
    await AsyncStorage.clear()
  } catch(e) {
    console.error("clear Data Error");
    return ;
  }

  console.log('Done.')
}

const TEST_SET_ITEM = () => {
  let multi_set_pairs = [
    ['UID234', "UID234_object"],
    ['UID345', "UID345_object"]
  ];
  storeData("key1", "value1")
  storeData("key2", "value2")
  storeData("key3", "value3")
  storeData("key4", "value4")
  storeData("key5", "value5")
  storeRandomData();
  multiStore(multi_set_pairs)
} 


const TEST_GET_ITEM = () =>{
  getData("UID234");
  _retrieve_100_Data();
  _retrieveData("key2");
  _retrieveData("key50");
  multiGetAsync(['key1', 'key3',"key3"]);
  getAsyncData("key28");
}

const TEST_REMOVE_ITEM =() =>{
  removeValue("key78");
  removeValue("key2");
  multipleRemove([keys[1],keys[2]]);
}

class SimpleViewApp extends Component {
  constructor(props) {
    super(props)
    this.state = {
      keys: []
    }
    TEST_SET_ITEM();
    TEST_GET_ITEM();
    TEST_REMOVE_ITEM();
    TEST_GET_ALL_KEYS();
    TEST_CLEAR_DATA();
  }
    logEvent1 = async () => {
        console.log("----------------calling logEvent1");
    }
    
    onPress = () => {
      console.log("onTouch---------------------")
      this.logEvent1();
    }
    
    componentDidMount() {
    }

render(){
        return(
<View style={styles.container}>
<TouchableHighlight onPress={this.onPress}>
<View style={styles.button}>
<Text>Touch Here</Text>
</View>
</TouchableHighlight>
<Text style={styles.text1}>TESTING OF @react-native-async-storage!!</Text>
             {this.state.keys.map((i)=>{let key= getObjKey();return (<Text key={key} style={styles.text2}>Obj-{key}: {i}</Text>)})}
</View>
        );
    }
};

const styles = StyleSheet.create({
  container: {
    width: 500,
    height: 500,
    backgroundColor: "yellow",
    display: "flex",
    alignItems: 'center',
    justifyContent: 'center'
  },
  text1: {
    fontSize: 25,
    color: "red"
  },
  text2: {
    fontSize: 20,
    color: "blue"
  },
  button: {
    alignItems: "center",
    backgroundColor: "#DDDDDD",
    padding: 10
  },

});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
