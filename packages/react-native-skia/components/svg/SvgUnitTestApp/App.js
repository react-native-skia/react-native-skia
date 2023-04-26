import * as React from "react";
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  Dimensions,
  TouchableHighlight
} from "react-native";

import MainComponent from "./MainComponent";

import ViewNames,{AppName} from "./Config";

const windowSize = Dimensions.get('window');

var pageName = AppName;
const SimpleViewApp = () => {
  const [page, setPage] = React.useState(-1);

  const setPageCallback = (page) => {
    setPage(page);
    pageName = AppName;
  };

  const loadModule = (index) => {
    pageName = ViewNames[index].name;
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
      <Text style={styles.AppTitle}>{pageName}</Text>
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
    marginLeft:10,
    marginTop:10
  },
  controlItems: {
    marginHorizontal: 10,
    marginVertical: 5,
    alignItems: "center",
    justifyContent: "center",
    width: 250,
    height: 50,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  },
  AppTitle:{
    fontSize:18,
    fontWeight:'bold',
    textAlign:'center' ,
    color:'purple'
  }
});

export default SimpleViewApp;

AppRegistry.registerComponent("SimpleViewApp", () => SimpleViewApp);
