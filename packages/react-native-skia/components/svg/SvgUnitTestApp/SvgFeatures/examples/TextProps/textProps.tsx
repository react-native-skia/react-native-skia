import React, {Component} from 'react';
import {Svg, G, Text, TSpan,LinearGradient,Defs,Stop} 
from 'react-native-svg';

const startXOffset='10';
const startYOffset='10';
const fill="yellow";
const stroke="green";
const fontSize='22';
const fontFamily='DejaVuSans-Bold';
const wordSpacing='10';
const LetterSpacing='3';
const yoffset=22+5;
const Long_Text='Svg does not support paragraph.It means content supposed to write in a single line.';

class text_anchor extends Component {
  static title = 'Text Anchor';
  render() {
    return (
     <Svg height='90' width="800"  >
        <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='100' dy={yoffset} textAnchor='start'>textAnchor : Start</TSpan>
          <TSpan x='100' dy={yoffset} textAnchor='middle'>textAnchor : Middle</TSpan>
          <TSpan x='100' dy={yoffset} textAnchor='end'>textAnchor : End</TSpan>
        </Text>
    </Svg>
);
}
}

class text_decoration extends Component {
  static title = 'Text Decoration';
  render() {
    return (
     <Svg height='110' width="800"  >
      <Text fill={fill} fontSize='20' fontFamily={fontFamily} >
          <TSpan x='50' dy={yoffset} textDecoration='none'>textDecoration : None</TSpan>
          <TSpan x='50' dy={yoffset} textDecoration='underline'>textDecoration : Underline</TSpan>
          <TSpan x='50' dy={yoffset} textDecoration='line-through'>textDecoration : Line Through</TSpan>
          <TSpan x='50' dy={yoffset} textDecoration='overline'>textDecoration : Overline</TSpan>
        </Text>
    </Svg>
);
}
}

class letter_Spacing extends Component {
  static title = 'Letter Spacing';
  render() {
    return (
     <Svg height="250" width="800"  >
      <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='50' dy={yoffset} letterSpacing='3'>Letter Spacing as 3px </TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='1'>Letter Spacing as 1px</TSpan>
          <TSpan x='50' dy={yoffset} >Letter Spacing as defaut</TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='0'>Letter Spacing as 0</TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='-1'>Letter Spacing as -1</TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='-3'>Letter Spacing as -3</TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='.5'>Letter Spacing as .5px</TSpan>
          <TSpan x='50' dy={yoffset} letterSpacing='10'>Letter Spacing as 10px</TSpan>
        </Text>
    </Svg>
);
}
}

class word_Spacing extends Component {
  static title = 'Word Spacing';
  render() {
    return (
     <Svg height="90" width="800"  >
      <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='50' dy={yoffset} >Word Spacing as default</TSpan>
          <TSpan x='50' dy={yoffset} wordSpacing='50'>Word Spacing as 50 px</TSpan>
          <TSpan x='50' dy={yoffset} wordSpacing='0'>Word Spacing as 0</TSpan>
        </Text>
    </Svg>
);
}
}

class font_weight extends Component {
  static title = 'Font Weight';
  render() {
    return (
     <Svg height="120" width="800"  >
      <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='50' dy={yoffset} fontWeight='bold'>Font Weight as bold</TSpan>
          <TSpan x='50' dy={yoffset} fontWeight='normal'>Font Weight as normal</TSpan>
          <TSpan x='50' dy={yoffset} fontWeight='800'>Font Weight as 800</TSpan>
          <TSpan x='50' dy={yoffset} fontWeight='100'>Font Weight as 100</TSpan>
        </Text>
    </Svg>
);
}
}

class font_style extends Component {
  static title = 'Font Style';
  render() {
    return (
     <Svg height="90" width="800"  >
      <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='50' dy={yoffset} textDecoration='normal'>Font Style as normal</TSpan>
          <TSpan x='50' dy={yoffset} fontStyle='italic'>Font Style as italic</TSpan>
          <TSpan x='50' dy={yoffset} fontStyle='oblique'>Font Style as oblique</TSpan>
        </Text>
    </Svg>
);
}
}

class deltaPositioning extends Component {
  static title = 'Delta Positioning';
  render() {
    return (
     <Svg height="220" width="800"  >
      <Text fill={fill} fontSize='18' fontFamily={fontFamily} >
          <TSpan x='50' y='70' fill='blue'>1. Text @ x=50 y=50</TSpan>
          <TSpan dy='22'>2. Text @ dy=20 </TSpan>
          <TSpan dx='22' >3. Text @ dx=20</TSpan>
          <TSpan x='0' y='130' fill='blue'>4. Text @ x=0 y=150</TSpan>
          <TSpan dy='22'>5.Text @ dy=20 </TSpan>
          <TSpan dx='22'>6.Text @ dx=20 </TSpan>
          <TSpan x='0' y='15' fill='blue'>7. Text @ x=0 y=15</TSpan>
          <TSpan dy='22'>8.Text @ dy=20 </TSpan>
          <TSpan dx='22'>9.Text @ dx=20 </TSpan>
        </Text>
    </Svg>

);
}
}

class text_style extends Component {
  static title = 'Text with Stroke & Fill';
  render() {
    return (
     <Svg height="250" width="800"  >
        <Defs>
          <LinearGradient id="grad-horz" x1="0%" y1="0%" x2="100%" y2="100%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="1" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
          <LinearGradient id="grad-vert" x1="0%" y1="0%" x2="0%" y2="100%">
            <Stop offset="0%" stopColor="rgb(255,255,0)" stopOpacity="1" />
            <Stop offset="100%" stopColor="red" stopOpacity="1" />
          </LinearGradient>
        </Defs>
        <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x='50' dy='32' fontSize='30'>Text With Fill</TSpan>
          <TSpan x='50' dy='34' fontSize='30' fill="url(#grad-horz)">Text With LinearGradient Fill</TSpan>
          <TSpan x='50' dy='36' fill='none' stroke={stroke} fontSize='32' strokeWidth='2'>Text With stroke</TSpan>
          <TSpan x='50' dy='35' fill='none' stroke="url(#grad-horz)" fontSize='32' strokeWidth='2'>Text With Linear Gradient stroke</TSpan>
          <TSpan x='50' dy='39' stroke={stroke} fontSize='45' strokeWidth='3'>Text With Fill & Stroke</TSpan>
          <TSpan x='50' dy='48' fill='blue' stroke='yellow' fontSize='40' strokeWidth='2' strokeLinecap="square" strokeDasharray="2,4">Text With Fill & Stroke styled</TSpan>
        </Text>
    </Svg>
);
}
}

class full_set extends Component {
  static title = 'Combined Feature with Inheritence';
  render() {
    return (
     <Svg height="250" width="800"  >
      <Text fill='yellow' fontSize={fontSize} fontFamily={fontFamily} 
            letterSpacing='5'
            textAnchor='start'
            textDecoration='underline'
            x='100'
            dy='50'
            >
          <TSpan >Style from Parent    </TSpan>
          <TSpan x='100' dy='40' textDecoration='line-through'>
                 Text Decoration changed to line-through
          </TSpan>
          <TSpan x='100' dy='40' textAnchor='middle'>
                 Text Anchor changed to middle
          </TSpan>
          <TSpan x='100' dy='40' stroke='green'>
                 fill color changed to green
          </TSpan>
          <TSpan x='100' dy='40' fontFamily='monospace'>
                 font family changed to monospace
          </TSpan>
        </Text>
    </Svg>
);
}
}

class paragrph_test extends Component {
  static title = 'Long Test with delta positioning';
  render() {
    return (
     <Svg height="200" width="800"  >
      <Text fill={fill} fontSize={fontSize} fontFamily={fontFamily} >
          <TSpan x= '0' dy='50' fill='yellow'>{Long_Text}</TSpan>
          <TSpan x= '0' dy='50'>2. TSpan with delta x= '0' dy='50' </TSpan>
          <TSpan dx='50' >3. TSpan with delta dx='50'</TSpan>
          <TSpan x= '0' dy='50' fill='yellow' fontSize='18'>{Long_Text}</TSpan>
        </Text>
    </Svg>

);
}
}

const features = [text_style,font_weight,font_style,text_anchor,letter_Spacing,word_Spacing,text_decoration,deltaPositioning,full_set,paragrph_test];

export default features;

