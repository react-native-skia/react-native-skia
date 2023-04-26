import * as React from "react";
import { StyleSheet, Text, View, TouchableHighlight } from "react-native";

const MainComponent = (props) => {
  console.log("MainComponent", props);

  const DynamicComponent = props.item.module;

  return (
    <View style={styles.mainView}>
      <View style={styles.headerView}>
        <BackButton backButtonCallback={props.backButtonCallback} />
        <View>
          <Text style={styles.headerText}>{props.item.name}</Text>
        </View>
      </View>
      <View style={styles.container}>
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
        onPress={() => backButtonCallback()}
      >
        <Text style={{ margin: 2 }}>Next</Text>
      </TouchableHighlight>
    </View>
  );
};

const styles = StyleSheet.create({
  headerView: {
    flexDirection: "column"
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
    width: 80,
    height: 40,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  },
    container: {
   flexDirection: 'row',
    paddingTop:10,
    paddingBottom:10,
    alignContent:'space-around',
    justifyContent:'space-around',
    backgroundColor: "black",
    borderColor:'green',
    borderWidth:3,
  },
});

export default MainComponent;
