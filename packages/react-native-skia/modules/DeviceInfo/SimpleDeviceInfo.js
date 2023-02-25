import * as React from "react";
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight
} from "react-native";
import DeviceInfo, { getBootloader } from "react-native-device-info";
const SimpleViewApp = () => {
  const [responseText, setResponseText] = React.useState("React-native Device info Api's");
  const GetSystemName = () => {
    let responseString =
    "Device Model: " +DeviceInfo.getModel()+"\n" +
    "Device ID: " +DeviceInfo.getDeviceId() +"\n" +
    "Version: " +DeviceInfo.getVersion() +"\n" +
    "System Version: " +DeviceInfo.getSystemVersion() +"\n" +
    "Tablet: " +DeviceInfo.isTablet() +"\n" +
    "Readable Version: " +DeviceInfo.getReadableVersion() +"\n" +
    "Application Name: " +DeviceInfo.getApplicationName() +"\n" +
    "Bundled ID: " +DeviceInfo.getBundleId() +"\n" +
    "Brand: " +DeviceInfo.getBrand() +"\n" +
    "Device Type:" +DeviceInfo.getDeviceType() +"\n" +
    "Device Name: " +DeviceInfo.getSystemName() +"\n" +
    "-------------------------------------------------" + "\n" +
    "Manufacture Sync:" +DeviceInfo.getManufacturerSync() +"\n" +
    "Land scape Sync:" +DeviceInfo.isLandscapeSync() +"\n" +
    "Emulator Sync: " +DeviceInfo.isEmulatorSync() +"\n" +
    "Device Carrier Sync:" +DeviceInfo.getCarrierSync() +"\n" +
    DeviceInfo.isEmulator()
    .then((result) => {
      responseString += "Emulator: " + result +"\n";
      setResponseText(responseString);
    })
    .catch((err) => {
      responseString += "Emulator error : "+ err +"\n";
      setResponseText(responseString);
    })
    DeviceInfo.getFreeDiskStorage()
    .then((result) => {
      responseString += "free Disk space: " + result +"\n";
      setResponseText(responseString);
    })
    .catch((err) => { 
      responseString += "free disk space error: ", err;
      setResponseText(responseString);
    })
    DeviceInfo.getManufacturer()
    .then((result) => {
      responseString += "Device Manufacturer : " + result + "\n";
      setResponseText(responseString);
    })
    .catch((err) => {
      responseString += "Device Manufacturer error: ", err;
      setResponseText(responseString);
    })
  }; 

  return (
    <View style={styles.mainView}>
      <View>
        <TouchableHighlight
          isTVSelectable="true"
          underlayColor="#FFBA08"
          style={styles.controlItems}
          onPress={GetSystemName}>
          <Text style={{ margin: 2, fontSize: 19,  }}>{"React-Native-Device-Info"}</Text>
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
    flex: 1
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