'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  AppRegistry,
} = require('react-native');

const NUM_ITEMS = 25;

class ScrollViewSimpleExample extends React.Component<{...}> {
  makeItems: (nItems: number, styles: any) => Array<any> = (
    nItems: number,
    styles,
  ): Array<any> => {
    const items = [];
    for (let i = 0; i < nItems; i++) {
      items[i] = (
          <View key={i} style={styles}>
            <Text>{'Item ' + i}</Text>
          </View>
        );
    }
    return items;
  };

  render(): React.Node {
    // One of the items is a horizontal scroll view
    const items = this.makeItems(NUM_ITEMS, styles.itemWrapper);

    const verticalScrollView = (
      <>
      <ScrollView style={styles.verticalScrollView}>{items}</ScrollView>
      <TouchableOpacity style={{margin:10,width:30,height:30,backgroundColor:'blue'}}>
      </TouchableOpacity>
      <TouchableOpacity style={{margin:10,width:30,height:30,backgroundColor:'orange'}}>
      </TouchableOpacity>
      </>
    );

    return verticalScrollView;
  }
}

const styles = StyleSheet.create({
  verticalScrollView: {
    margin: 10,
    backgroundColor : 'pink',
    borderWidth: 5,
    borderColor: 'black',
    width:500,
    height:500,
    shadowColor:'green',
    shadowOffset:{width:10,height:10},
    shadowRadius:10,
    shadowOpacity:0.5
  },
  itemWrapper: {
    backgroundColor: '#dddddd',
    alignItems: 'center',
    borderRadius: 5,
    borderWidth: 5,
    borderColor: '#a52a2a',
    padding: 30,
    margin: 5,
  }
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
