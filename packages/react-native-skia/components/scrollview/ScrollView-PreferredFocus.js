'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  Text,
  View,
  TouchableHighlight,
  Pressable,
  AppRegistry,
  TextInput
} = require('react-native');

const NUM_ITEMS = 20;

const testSingleTi = false;
const testMultiTi = !false;
const testLastItemFocus = false;
const testDynamicUpdate = !false;

class ScrollViewSimpleExample extends React.Component<{...}> {

  constructor(props) {
   super(props);
   this.state={focusStatus:true,width:200,count:1};
  }

  makeItems: (nItems: number, styles: any) => Array<any> = (
    nItems: number,
    styles,
  ): Array<any> => {
    const items = [];
    for (let i = 0; i <= nItems; i++) {
      if((i>12) && (i<20)) {
        items[i] = (
          <TouchableHighlight hasTVPreferredFocus={this.state.focusStatus} key={i} style={[styles,{width:this.state.width}]} onFocus={()=>{console.log("focus event :"+i);}} onBlur={()=>{console.log("blur event :"+i)}} onPress={()=>{}} underlayColor={"orange"}>
            <Text>{'Item ' + i}</Text>
          </TouchableHighlight>
        );
      } else {
        items[i] = (
          <TouchableHighlight key={i} style={styles} onPress={()=>{}} underlayColor={"yellow"}>
            <Text>{'Item ' + i}</Text>
          </TouchableHighlight>
        );
      }
    }

    if(testSingleTi || testMultiTi) {
      items[nItems+1] = (
        <TextInput style={{backgroundColor:'yellow',borderColor:'blue',borderWidth:2,width:400,height:400}} autoFocus={true} placeholder={"TextInput1WithAutoFocus"}>
        </TextInput>
      );
    }
    if(testMultiTi) {
      items[nItems+2] = (
        <TextInput style={{backgroundColor:'yellow',borderColor:'blue',borderWidth:2,width:400,height:400}} autoFocus={false} placeholder={"TextInput2NoAutoFocus"}>
        </TextInput>
      );
      items[nItems+3] = (
        <TextInput style={{backgroundColor:'yellow',borderColor:'blue',borderWidth:2,width:400,height:400}} autoFocus={true} placeholder={"TextInput3WithAutoFocus"}>
        </TextInput>
      );
    }
    if(testLastItemFocus) {
      items[nItems+4] = (
          <TouchableHighlight hasTVPreferredFocus={true} key={i} style={[styles,{width:350}]} onFocus={()=>{console.log("focus event last Item");}} onBlur={()=>{console.log("blur event last Item")}} onPress={()=>{}} underlayColor={"lightcyan"}>
            <Text>{'Item Last'}</Text>
          </TouchableHighlight>
      );
    }

    return items;
  };

  onScrollCallback = (e) => {
    console.log("onScroll event :" + e);
  }


  render(): React.Node {
    const items = this.makeItems(NUM_ITEMS, styles.itemWrapper);

    if(testDynamicUpdate) {
      setTimeout(()=>{
        if(this.state.count == 1) {
          this.setState({focusStatus:!this.state.focusStatus,width:200,count:2});
        } else if (this.state.count == 2) {
          this.setState({focusStatus:this.state.focusStatus,width:500,count:3});
        } else {
          this.setState({focusStatus:this.state.focusStatus,width:200,count:1});
        }
      },10000);
    }

    const comboItems = (
      <ScrollView style={styles.verticalScrollView} horizontal={true} onScroll={this.onScrollCallback}>{items}</ScrollView>
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
    width:800,
    height:400,
    //left:110,
    //top:30,
    //position:'absolute'
  },
  verticalScrollView_1: {
    margin: 5,
    backgroundColor : 'yellow',
    borderWidth: 5,
    borderColor: 'black',
    width:800,
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
    //padding: 30,
    margin: 5,
    height: 200,
  },
});

AppRegistry.registerComponent('SimpleViewApp', () => ScrollViewSimpleExample);
export default ScrollViewSimpleExample;
