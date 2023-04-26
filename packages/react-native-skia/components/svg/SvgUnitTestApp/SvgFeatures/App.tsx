
'use strict';

import React, {Component} from 'react';
import {
  Dimensions,
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  TouchableHighlight,
  ScrollView
} from 'react-native';
import { AppName } from '../Config';
import * as examples from './examples';

const hairline = StyleSheet.hairlineWidth;
const windowSize = Dimensions.get('window');

const styles = StyleSheet.create({
  container: {
    paddingTop: 5,
    borderWidth:2, 
    borderColor: "green",
    width:windowSize.width*.95,
  },
  contentContainer: {
    flexDirection:'row',
    flexWrap: 'wrap',
    marginHorizontal: 10,
    marginVertical: 5,
    alignItems: "center",
    justifyContent: "center",
  },
  verticalContainer: {
    flexDirection: 'row',
  },
  elements : {
    flexWrap: 'wrap',
    flexDirection: 'row',
    marginHorizontal: 5,
    marginVertical: 5,
    alignContent:'space-around',
    justifyContent:'space-around',
    width:windowSize.width*.95,
  },
  welcome: {
    padding: 5,
    color: '#f60',
    fontSize: 18,
    fontWeight: 'bold',
    textAlign:'center'
  },
  pageTitle : {
    padding: 10,
    color: 'purple',
    fontSize: 24,
    fontWeight: 'bold',
    textAlign:'center'
  },
  link: {
    width: 150,
    height: 50,
    backgroundColor: 'mediumpurple',
    marginHorizontal: 10,
    marginVertical: 5,
    alignItems: "center",
    justifyContent: "center",
    borderColor: "black",
    borderWidth: 2,
  },
  title: {
    marginLeft: 10,
  },
  cell: {
    height: 40,
    flexDirection: 'row',
    backgroundColor: 'transparent',
    alignItems: "center",
    justifyContent: "center",
  },
  example: {
    paddingVertical: 5,
    alignItems: 'center',
    borderBottomWidth: hairline,
    borderBottomColor: '#ccc',
  },
  sampleTitle: {
    marginHorizontal: 15,
    fontSize: 16,
    color: '#666',
  },
  controlItems: {
    marginHorizontal: 15,
    marginVertical: 5,
    alignItems: "center",
    justifyContent: "center",
    width: 180,
    height: 40,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  }
});

const names: (keyof typeof examples)[] = [
  'fill',
  'stroke',
  'stroking',
  'viewbox',
  'transform',
  'TextProps'
];


const initialState = {
  content: null,
  page   : -1,
  pageName:'SVG Demo for React Native Skia',
};

export default class SvgExample extends Component {
state: {
  content: React.ReactNode;
  page: -1;
  pageName:'SVG Demo for React Native Skia';
} = initialState;

show = (index,name: keyof typeof examples) => {
  let example = examples[name];
  this.prepareExampleLoad(index,name)
  if (example) {
    let samples = example.samples;
    this.setState({
      content: (
        <View style={styles.elements}>
          {samples.map((Sample, i) => (
            <View style={styles.example} key={`sample-${i}`}>
             <Text style={styles.sampleTitle}>{Sample.title}</Text>
             <Sample />
            </View>
          ))}
        </View>
      ),
    });
  }
};

prepareExampleLoad =(index,pageName)=> {
  this.setState({page:index,pageName:pageName})
}


BackButton = () => {
return (
  <View>
    <TouchableHighlight
      isTVSelectable="true"
      underlayColor="#FFBA08"
      style={styles.controlItems}
      onPress={() => this.prepareExampleLoad(-1,'SVG Demo for React Native Skia')}
    >
      <Text style={{ margin: 2 }}>Back</Text>
    </TouchableHighlight>
  </View>
  );
};

NextButton = () => {
  var index=(this.state.page+1)%(names.length);
  return (
    <View>
      <TouchableHighlight
        isTVSelectable="true"
        underlayColor="#FFBA08"
        style={styles.controlItems}
        onPress={() => {this.show(index,names[index])}}>
        <Text style={{ margin: 2 }}>Next</Text>
      </TouchableHighlight>
    </View>
  );
};

getExamples = () => {
  var index = -1;
  return names.map(name => {
    let example = examples[name];
    index=index+1;
    return (
      <TouchableOpacity
        style={styles.link}
        underlayColor="#ccc"
        key={`example-${name}`}
        onPress={() => {
        this.show(index,name)}}>
        <View style={styles.cell}>
          <Text style={styles.title}>{name}</Text>
        </View>
      </TouchableOpacity>
    );
  });
};

showMainPage =() => {
  return (
    <View style={styles.container}>
      <Text style={styles.welcome}>{this.state.pageName}</Text>
      <View style={styles.verticalContainer}>
         <View style={styles.contentContainer}>{this.getExamples()}</View>
      </View>
    </View>
  );
};

showExamples =() => {
  return (
    <View>
      {this.BackButton()}
      <View style={styles.container}>
        <Text style={styles.pageTitle}>{this.state.pageName}</Text>
        <View style={styles.verticalContainer}>
         {this.state.content}
        </View>
      </View>
      {this.NextButton()}
    </View>
  );
};

render() {
  if(this.state.page == -1) return this.showMainPage()
  else return this.showExamples()
}

}
