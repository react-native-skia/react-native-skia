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
      if(!(i%8)) {
         items[i] = (
          <View key={i} style={{ backgroundColor: 'red', alignItems: 'center',borderRadius: 5, borderWidth: 5, borderColor: '#a52a2a', padding: 30, margin: 5, height:650}}>
            <Text>{'Item ' + i}</Text>
          </View>
         );
      } else if ( !(i % 5)) {
         items[i] = (
          <View key={i} style={{backgroundColor: 'orange', alignItems: 'center',borderRadius: 5, borderWidth: 5, borderColor: '#a52a2a', padding: 30, margin: 5}}>
            <Text>{'Item ' + i}</Text>
          </View>
         );
      } else {
      items[i] = (
          <TouchableOpacity key={i} style={styles}>
            <Text>{'Item ' + i}</Text>
          </TouchableOpacity>
        );
      }
    }
    return items;
  };

  render(): React.Node {
    const items = this.makeItems(NUM_ITEMS, styles.itemWrapper);

    const comboItems = (
      <>
      <ScrollView style={styles.verticalScrollView}>{items}</ScrollView>
      <ScrollView style={styles.verticalScrollView_1}>{items}</ScrollView>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:10,left:10}}>
      </TouchableOpacity>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:300,left:10}}>
      </TouchableOpacity>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:450,left:10}}>
      </TouchableOpacity>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:10,left:320}}>
      </TouchableOpacity>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:300,left:320}}>
      </TouchableOpacity>
      <TouchableOpacity style={{width:80,height:50,backgroundColor:'green',position:'absolute',top:450,left:320}}>
      </TouchableOpacity>
      </>
    );

    return comboItems;
  }
}

const styles = StyleSheet.create({
  verticalScrollView: {
    margin: 5,
    backgroundColor : 'pink',
    borderWidth: 5,
    borderColor: 'black',
    width:200,
    height:400,
    left:110,
    top:30,
    position:'absolute'
  },
  verticalScrollView_1: {
    margin: 5,
    backgroundColor : 'yellow',
    borderWidth: 5,
    borderColor: 'black',
    width:200,
    height:400,
    left:420,
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
    height: 250,
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
