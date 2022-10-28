import * as React from 'react';
import { AppRegistry, Text, View } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
    <View>
      <View style={{ margin:10, backgroundColor:'red' }}>
        <Text>Default LetterSpacing: The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'green' }}>
        <Text style={{ letterSpacing:4 }}>LetterSpacing: 4 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'blue' }}>
        <Text style={{ letterSpacing:10.5 }}>LetterSpacing: 10.5 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'red' }}>
        <Text style={{ letterSpacing:15 }}>LetterSpacing: 15 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'green' }}>
        <Text style={{ letterSpacing:0 }}>LetterSpacing: 0 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'blue' }}>
        <Text style={{ letterSpacing:-5 }}>LetterSpacing: -5 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
      <View style={{ margin:10, backgroundColor:'red' }}>
        <Text style={{ letterSpacing:-3.5 }}>LetterSpacing: -3.5 The letter-spacing CSS property sets the horizontal spacing behavior between text characters. This value is added to the natural spacing between characters while rendering the text. Positive values of letter-spacing causes characters to spread farther apart, while negative values of letter-spacing bring characters closer together.</Text>
      </View>
    </View>
  );
}
  
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
