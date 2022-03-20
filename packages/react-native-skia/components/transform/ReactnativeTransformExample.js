import * as React from 'react';
import { useState } from 'react';
import { View, AppRegistry, Image, Text,StyleSheet } from 'react-native';

const SimpleViewApp = React.Node = () => {

const styles = StyleSheet.create({
  container: {
    flex: 1,
     alignItems: "center",
  },
  box: {
    height: 100,
    width: 100,
    borderRadius: 5,
    marginVertical: 40,
    backgroundColor: '#61dafb',
    alignItems: 'center',
    justifyContent: 'center',
  },
  text: {
    fontSize: 14,
    fontWeight: 'bold',
    margin: 8,
    color: '#000',
    textAlign: 'center',
  },
});

const renderMainView = () => {

    const [Count, toggleState] = useState(true);
	const timer = setTimeout(()=>{
        toggleState((Count+1)%5);
    }, 4000)
   if(Count <= 1) {
   return (
      <View style={{flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',}}>
      <View style={styles.box}>
          <Text style={styles.text}>Original Object</Text>
      </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ scale: 2 }],
            },
          ]}>
          <Text style={styles.text}>Scale by 2</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ scaleX: 2 }],
            },
          ]}>
          <Text style={styles.text}>ScaleX by 2</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ scaleY: 2 }],
            },
          ]}>
          <Text style={styles.text}>ScaleY by 2</Text>
        </View>
    </View>
    );
 }
 else if (Count == 2) {
 	return (
      <View style={{flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',}}>
      <View style={styles.box}>
          <Text style={styles.text}>Original Object</Text>
      </View>
        <View
          style={[
            styles.box,
            {
              transform: [{ rotate: '45deg' }],
            },
          ]}>
          <Text style={styles.text}>Rotate by 45 deg</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ rotateX: '45deg' }, { rotateZ: '45deg' }],
            },
          ]}>
          <Text style={styles.text}>Rotate X&Z by 45 deg</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ rotateY: '45deg' }, { rotateZ: '45deg' }],
            },
          ]}>
          <Text style={styles.text}>Rotate Y&Z by 45 deg</Text>
        </View>
</View>
);
}
 else if (Count == 3) {
 	return (
      <View style={{flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',}}>
      <View style={styles.box}>
          <Text style={styles.text}>Original Object</Text>
      </View>
        <View
          style={[
            styles.box,
            {
              transform: [{ skewX: '45deg' }],
            },
          ]}>
          <Text style={styles.text}>SkewX by 45 deg</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ skewY: '45deg' }],
            },
          ]}>
          <Text style={styles.text}>SkewY by 45 deg</Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ skewX: '30deg' }, { skewY: '30deg' }],
            },
          ]}>
          <Text style={styles.text}>Skew X&Y by 30 deg</Text>
        </View>
</View>
);
}
 else if (Count == 4) {
 	return (
      <View style={{flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',}}>
      <View style={styles.box}>
          <Text style={styles.text}>Original Object</Text>
      </View>
        <View
          style={[
            styles.box,
            {
              transform: [{ translateX: -50 }],
            },
          ]}>
          <Text style={styles.text}>TranslateX by -50 </Text>
        </View>

        <View
          style={[
            styles.box,
            {
              transform: [{ translateY: 50 }],
            },
          ]}>
          <Text style={styles.text}>TranslateY by 50 </Text>
        </View>
        </View>
  );
 }            
}
return (
   renderMainView()
  );  

};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
