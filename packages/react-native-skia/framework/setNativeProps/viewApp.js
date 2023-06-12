import React, { useRef } from 'react';
import { View, Text, TouchableOpacity, AppRegistry} from 'react-native';

const MyviewApp = () => {
  const myViewRef = useRef(null);

  const handleButtonClick0 = () => {
    if (myViewRef.current) {
      myViewRef.current.setNativeProps({
        style: { backgroundColor: 'red'},
      });
    }
  };

  const handleButtonClick1 = () => {
    if (myViewRef.current) {
      myViewRef.current.setNativeProps({
        style: { top: 500},
      });
    }
  };

  const handleButtonClick2 = () => {
    if (myViewRef.current) {
      myViewRef.current.setNativeProps({
        style: {   height:100},
      });
    }
  };

  const handleButtonClick3 = () => {
    if (myViewRef.current) {
      myViewRef.current.setNativeProps({
        style: { opacity:0.5},
      });
    }
  };

  const handleButtonClick4 = () => {
    if (myViewRef.current) {
      myViewRef.current.setNativeProps({
        style: { transform: [{ scaleX:2 }, { scaleY : 4 }]},
      });
    }
  };

  return (
    <View style={styles.container}>
      <View ref={myViewRef} style={styles.box} />
      <Text style={styles.text}>Click the button to change in setNativeProps</Text>
      <TouchableOpacity style={styles.button} onPress={handleButtonClick0}>
        <Text style={styles.buttonText}>Change Color</Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.button} onPress={handleButtonClick1}>
        <Text style={styles.buttonText}>Change Top</Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.button} onPress={handleButtonClick2}>
        <Text style={styles.buttonText}>Change Height</Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.button} onPress={handleButtonClick3}>
        <Text style={styles.buttonText}>Change Opacity </Text>
      </TouchableOpacity>

      <TouchableOpacity style={styles.button} onPress={handleButtonClick4}>
        <Text style={styles.buttonText}>Change Transform</Text>
      </TouchableOpacity>
    </View>
  );
};

const styles = {
  container: {
    flex: 1,
  },
  box: {
    width: 200,
    height: 200,
    backgroundColor: 'blue',
    marginBottom: 100,
  },
  text: {
    fontSize: 16,
    marginBottom: 10,
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
};

export default MyviewApp;

