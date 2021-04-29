import * as React from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {

  return (
    <View
      style={{ flex: 1,
               flexDirection: 'column',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: 'white' }}>
      <Text style={{ marginTop:'2' }}>
	  NORMAL TEXT : REACT NATIVE LOVES SKIA
      </Text>
      <Text style={{ fontSize: 10,textAlign: 'center' , marginTop:'2'}}>
        PARAGRAPH TEST 1: DEFAULT FONT COLOR - FONT SIZE 10 - CENTER TEXT ALIGN - WIDTH DEF: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

      <Text style={{ fontSize: 12,textAlign: 'center' , marginTop:'2'}} >
	PARAGRAPH TEST 2: DEFAULT FONT COLOR - FONT SIZE 12 - CENTER TEXT ALIGN - WIDTH DEF: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

      <Text style={{fontSize: 12,textAlign: 'left' , marginTop:'2'}} >
	PARAGRAPH TEST 3: DEFAULT FONT COLOR - FONT SIZE 12 - LEFT TEXT ALIGN - WIDTH DEF: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

      <Text style={{fontSize: 12,textAlign: 'right',marginTop:'2'}} >
	PARAGRAPH TEST 4: DEFAULT FONT COLOR - FONT SIZE 12 - RIGHT TEXT ALIGN - WIDTH DEF: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

      <Text style={{fontSize:12,textAlign: 'left',width:'500',marginTop:'2'}} >
	PARAGRAPH TEST 5: DEFAULT FONT COLOR - FONT SIZE 12 - LEFT TEXT ALIGN - WIDTH 50: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

      <Text style={{fontSize:12,textAlign: 'left',width:'500',height:'70',marginTop:'5'}} >

	PARAGRAPH TEST 6: DEFAULT FONT COLOR - FONT SIZE 12 - LEFT TEXT ALIGN - WIDTH 50 - HEIGHT 70: React Native is like React, but it uses native components instead of web components as building blocks. So to understand the basic structure of a React Native app, you need to understand some of the basic React concepts, like JSX, components, state, and props. If you already know React, you still need to learn some React-Native-specific stuff, like the native components. This tutorial is aimed at all audiences, whether you have React experience or not.
      </Text>

    </View>
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
