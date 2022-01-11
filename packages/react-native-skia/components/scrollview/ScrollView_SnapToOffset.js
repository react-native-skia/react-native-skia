import React, { Component } from 'react';
import { View, StyleSheet, ScrollView, Dimensions,Text,TouchableOpacity,AppRegistry } from 'react-native';

const { width } = Dimensions.get('window');


class App extends Component {

  render() {

    const itemsCount = 50

    return (
      <ScrollView 
        ref={(scrollView) => { this.scrollView = scrollView; }}
        style={styles.container}
        snapToOffsets={[0,200,600,800,1100,1500,2100,2800,3600,4400]}
        >
        {[...Array(itemsCount)].map((x, i) =>
          <View style={[styles.view, {backgroundColor: i % 2 == 0 ? 'turquoise' : 'magenta',}]} >
		    <Text>{'Item ' + i}</Text>
	  </View>
        )}


      </ScrollView>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    margin : 100,  
    width : 500,
    height : 500,
    backgroundColor :  'yellow',	
  },
  view: {
    width: 100,
    height: 100,
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => App);

