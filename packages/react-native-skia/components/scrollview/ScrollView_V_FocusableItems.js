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
          <TouchableOpacity key={i} style={styles}>
            <Text>{'Item ' + i}</Text>
          </TouchableOpacity>
        );
    }
    return items;
  };

  render(): React.Node {
    const items = this.makeItems(NUM_ITEMS, styles.itemWrapper);

    const verticalScrollView = (
      <>
      <ScrollView style={styles.verticalScrollView}>{items}</ScrollView>
      <ScrollView style={styles.verticalScrollView_1}>{items}</ScrollView>
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
    width:200,
    height:500,
    left:10,
    top:30,
    position:'absolute'
  },
  verticalScrollView_1: {
    margin: 10,
    backgroundColor : 'yellow',
    borderWidth: 5,
    borderColor: 'black',
    width:200,
    height:500,
    left:300,
    top:30,
    position:'absolute'
  },
  itemWrapper: {
    backgroundColor: 'blue',
    alignItems: 'center',
    borderRadius: 5,
    borderWidth: 5,
    borderColor: '#a52a2a',
    padding: 30,
    margin: 5,
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
