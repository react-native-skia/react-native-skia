import * as React from "react";
import { StyleSheet, Text, View, TouchableHighlight, AppRegistry } from "react-native";

const MainComponent = (props) => {
  console.log("MainComponent", props);

  const DynamicComponent = props.item.module;

  return (
    <View style={styles.mainView}>
      <View style={styles.headerView}>
        <BackButton backButtonCallback={props.backButtonCallback} />
        <View>
          <Text styles={styles.headerText}>{props.item.name}</Text>
        </View>
      </View>
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
        <Text style={{ margin: 4, }}>Back to Home</Text>
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
    alignItems: "center",
    justifyContent: "center"
  },
  controlItems: {
    margin: 18,
    alignItems: "center",
    justifyContent: "center",
    width: 100,
    height: 50,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  }
});

export default MainComponent;


