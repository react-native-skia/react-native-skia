import * as React from "react";
import { StyleSheet, Text, View, TouchableHighlight } from "react-native";

const MainComponent = (props) => {
  console.log("MainComponent", props);

  const DynamicComponent = props.item.module;

  return (
    <View style={styles.mainView}>
      <BackButton backButtonCallback={props.backButtonCallback} />
      <View>
        <DynamicComponent />
      </View>
    </View>
  );
};

const BackButton = ({ backButtonCallback }) => {
  return (
    <View>
      <TouchableHighlight
        isTVSelectable="true"
        underlayColor="#FFBA08"
        style={styles.controlItems}
        onPress={() => backButtonCallback(-1)}
      >
        <Text style={{ margin: 2 }}>Home Page</Text>
      </TouchableHighlight>
    </View>
  );
};

const styles = StyleSheet.create({
  headerView: {
    flexDirection: "row"
  },
  headerText: {
    fontSize: 24,
    fontWeight:'bold',
    color:'purple',
    textAlign :'left'
  },
  controlItems: {
    margin: 18,
    alignItems: "center",
    justifyContent: "center",
    width: 180,
    height: 40,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  }
});

export default MainComponent;
