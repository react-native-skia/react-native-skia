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

const NUM_ITEMS = 250;

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

    const horizontalScrollView = (
      <>
      <ScrollView horizontal={true} style={styles.horizontalScrollView}>{items}</ScrollView>
      <ScrollView horizontal={true} style={styles.horizontalScrollView_1}>{items}</ScrollView>
      </>
    );

    return horizontalScrollView;
  }
}

const styles = StyleSheet.create({
  horizontalScrollView: {
    margin: 10,
    backgroundColor : 'pink',
    borderWidth: 5,
    borderColor: 'black',
    width:500,
    height:200,
    top:10,
    left:30,
    position:'absolute'
  },
  horizontalScrollView_1: {
    margin: 10,
    backgroundColor : 'yellow',
    borderWidth: 5,
    borderColor: 'black',
    width:500,
    height:200,
    top:300,
    left:30,
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
    width: 200,
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
