import * as React from "react";
import {
  // AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight
} from "react-native";

import MainComponent from "./MainComponent";

import ViewNames from "./Config";

const SimpleViewApp = () => {
  const [page, setPage] = React.useState(-1);

  const setPageCallback = () => { 
    var index = (page +1 ) % (ViewNames.length)
    setPage(index);
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
              <Text style={{ margin: 2 }}>{item.name}</Text>
            </TouchableHighlight>
          );
        })}
      {page !== -1 && showMainComponent()}
    </View>
  );
};

const styles = StyleSheet.create({
  mainView: {
    flexDirection:'row',
    flexWrap: 'wrap',
    marginHorizontal: 10,
    alignItems: "center",
    justifyContent: "center",
  },
  controlItems: {
    marginHorizontal: 10,
    marginVertical: 5,
    alignItems: "center",
    justifyContent: "center",
    width: 150,
    height: 50,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "mediumpurple"
  }
});

const samples=[SimpleViewApp ];

export {samples};

// AppRegistry.registerComponent("SimpleViewApp", () => SimpleViewApp);
