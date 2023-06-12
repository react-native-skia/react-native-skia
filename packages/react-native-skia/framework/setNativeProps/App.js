import * as React from "react";
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight
} from "react-native";

import MainComponent from "./MainComponent";

import ViewNames from "./modules";

const SimpleViewApp = () => {
  const [page, setPage] = React.useState(-1);

  const setPageCallback = (page) => {
    setPage(page);
  };

  const loadModule = (index) => {
    setPage(index);
  };

  const showMainComponent = () => {
    return (
      <MainComponent
        page={page}
        item={ViewNames[page]}
        backButtonCallback={setPageCallback}
      />
    );
  };

  return (
    <View style={styles.mainView}>
      <Text style={styles.HeaderText}>SetNativeProps</Text>
      {page === -1 &&
        ViewNames.map((item, index) => {
          return (
            
            <TouchableHighlight
              key={"view" + index}
              isTVSelectable="true"
              underlayColor="#FFBA08"
              style={styles.controlItems}
              onPress={() => loadModule(index)}
            >
              <Text style={styles.TextItem}>{item.name}</Text>
            </TouchableHighlight>
          );
        })}
      {page !== -1 && showMainComponent()}
    </View>
  );
};

const styles = StyleSheet.create({
  mainView: {
    flex: 1
  },
  controlItems: {
    margin: 25,
    alignItems: "center",
    justifyContent: "center",
    width: 250,
    height: 60,
    borderRadius: 8,
    borderColor: "floralwhite",
    borderWidth: 2,
    paddingHorizontal: 8,
    backgroundColor: "dodgerblue"
  },
  TextItem: {
    margin: 2,
    fontSize: 16,
    color: "floralwhite",
    fontWeight: "bold",
    fontStyle: "italic"
  },
  HeaderText: {
    marginVertical: 25,
    fontWeight: "bold",
    textAlign: "center",
    fontSize: 25
  }
});

export default SimpleViewApp;
AppRegistry.registerComponent("SimpleViewApp", () => SimpleViewApp);
