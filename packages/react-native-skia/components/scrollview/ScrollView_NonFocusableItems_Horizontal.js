'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  Text,
  View,
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
      if(i < 10) {
      items[i] = (
          <View key={i} style={styles}>
            <Text>{'Item 0' + i}</Text>
          </View>
        );
      } else {
      items[i] = (
          <View key={i} style={styles}>
            <Text>{'Item ' + i}</Text>
          </View>
        );

      }
    }
    return items;
  };

  render(): React.Node {
    // One of the items is a horizontal scroll view
    const items = this.makeItems(NUM_ITEMS, styles.itemWrapper);

    const horizontalScrollView = (
      <ScrollView style={styles.horizontalScrollView} horizontal={true}>{items}</ScrollView>
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
  },
  itemWrapper: {
    backgroundColor: '#dddddd',
    alignItems: 'center',
    borderRadius: 5,
    borderWidth: 5,
    borderColor: '#a52a2a',
    padding: 30,
    margin: 2,
    width: 200,
  }
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
