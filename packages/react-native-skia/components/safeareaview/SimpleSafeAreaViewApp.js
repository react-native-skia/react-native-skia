import React, { useState } from 'react';
import { AppRegistry, View, SafeAreaView, ActivityIndicator, TextInput, Text, StyleSheet } from 'react-native';

const App = () => {
  const [inputText, setInputText] = useState('');

  const handleInputChange = (text) => {
    setInputText(text);
  };

  return (
    <SafeAreaView style={styles.container}>
        <Text style={styles.text}>Enter some text:</Text>
        <TextInput
          style={styles.input}
          value={inputText}
          onChangeText={handleInputChange}
        />
        <Text style={styles.text}>You entered: {inputText}</Text>
        <ActivityIndicator size="large" color="#0000ff" />
        <ActivityIndicator size="small" color="#ffff00" />
        <ActivityIndicator size="large" color="#ff0000" />
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#b2d8d8',
    justifyContent: 'center',
    alignItems: 'center'
  },
  text: {
    fontSize: 18,
    marginBottom: 10,
  },
  input: {
    width: '80%',
    height: 40,
    backgroundColor: '#ffffff',
    borderWidth: 1,
    borderColor: '#000000',
    paddingHorizontal: 10,
    marginBottom: 20,
  },
});

export default App;
AppRegistry.registerComponent('SimpleViewApp', () => App);
