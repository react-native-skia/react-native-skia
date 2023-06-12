import * as React from 'react';
import { Text, View, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';


export default function ScrollViewTest() {
  const ref0 = React.useRef(null);
  console.log("-------app--------------")

  const getScrollItems = (count) => {
    let items = []
    for(let i=0; i<count; i++){
      let x = (
        <View style={{width: 100, height: 100, backgroundColor: "lightblue", borderWidth:1, padding: 5, margin: 5, justifyContent: "center", alignItems: "center"}}>
          <Text>COUNT-{i}</Text>
        </View>
      )
      items.push(x)
    }
    return items;
  }

  const setScrolling = (enable) => {
    console.log("----------start---------===------")
    if (ref0.current) {
      ref0.current.setNativeProps({
        style: { backgroundColor: "yellow"},
        scrollEnabled: enable,
        indicatorStyle: "white",
        showsVerticalScrollIndicator: false

      });
    }
    console.log("----------end-----------------")
  }

  return (
    <View style={styles.container}>
      <Text style={styles.paragraph}>Test: ScrollView setNativeProps</Text>
      <TouchableOpacity style={styles.button} onPress={()=>{setScrolling(true)}}>
        <Text style={styles.buttonText}>Enable Scrolling</Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.button} onPress={()=>{setScrolling(false)}}>
        <Text style={styles.buttonText}>Disable Scrolling</Text>
      </TouchableOpacity>

      <ScrollView ref={ref0} style={styles.scrollContainer} scrollEnabled={true}
         contentContainerStyle={{justifyContent: "center", alignItems: "center"}}>
        {getScrollItems(10)}
      </ScrollView>

    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    justifyContent: 'center',
    alignItems: "center",
    backgroundColor: '#ecf0f1',
    padding: 8,
  },
  paragraph: {
    margin: 24,
    fontSize: 18,
    fontWeight: 'bold',
    textAlign: 'center',
  },
  scrollContainer: {
    width: 150,
    padding: 10,
    height: 250,
    backgroundColor: "lightgrey",
    borderWidth: 1
  },
  button: {
    margin: 10,
    alignItems: "center",
    justifyContent: "center",
    width: 160,
    height: 60,
    borderRadius: 70,
    borderColor: "floralwhite",
    borderWidth: 2,
    paddingHorizontal: 8,
    backgroundColor: "green"
  },
  buttonText: {
    color: 'white',
    fontSize: 16,
    fontWeight: 'bold',
  },
});
