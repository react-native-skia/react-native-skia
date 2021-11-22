import  React, { useState } from 'react';
import { View,Text,Image,StyleSheet,AppRegistry} from 'react-native';


var TestCaseTimeOut=5000;
var test="Combined";/*Translate , Scale , Roate, Skew ,Combined*/
var translateProps =[[]                 ,//original
                     [{translateY:-100}], //Move UP
                     [{translateY:100}],   //Move DOWN
                     [{translateX:-100}],  //Move Left
                     [{translateX:100}]  //Move Right
          ]
var translateTestCases =["No Transform","Move UP" ,"Move DOWN","Move Left","Move Right"];         

var scaleProps =[    []                 ,//original
                     [{scaleY:2}], //Vertical scalling
                     [{scaleX:2}],   //Horizontal Scalling 
                     [{scaleX:-1}],  //Horizontal Mirroring
                     [{scaleY:-1}]  //Vertical Mirroring
          ]
var scaleTestCases =["No Transform","Vertical scalling" ,"Horizontal Scalling","Horizontal Mirroring","Vertical Mirroring"];
var rotateProps =[   []                 ,//original
                     [{rotate: "45deg" }], //Rotate Clockwise
                     [{ rotate: "-45deg" }],   //Rotate Anti Clockwise
                     [{ rotateX: "60deg" }],  //Rotate X
                     [{ rotateY: "60deg" }],  //Rotate Y
          ]
var rotateTestCases =["No Transform","Rotate Clockwise" ,"Rotate Anti Clockwise","Rotate X Axis ","Rotate Y Axis "];
var skewProps =[     []                 ,//original
                     [{skewX:'45deg'}], //Vertical Skew Up
                     [{skewX:'-45deg'}],   //Vertical Skew Down
                     [{skewY:'45deg'}],  //Horizontal Skew Right
                     [{skewY:'-45deg'}]  //Horizontal Skew Left
          ]
var skewTestCases =["No Transform","Horizontal Skew Left" ,"Horizontal Skew Right","Vertical Skew Down","Vertical Skew Up"];    
var combinedProps =[     []                 ,//original
                     [{scaleX:2},{skewX:'45deg'},{rotate: "45deg"}], 
                     [{skewX:'45deg'},{rotate: "45deg"},{scaleX:2}],   
                     [{translateX:100},{skewX:'45deg'},{rotate: "45deg"},{scaleX:2}], 
                     [{skewX:'45deg'},{rotate: "45deg"},{scaleX:2},{translateX:100}]  
          ]
var combinedTestCases =["No Transform","Rotate, Skew & Scale","Scale,Rotate & Skew","Scale,Rotate, Skew & Translate" ,"Translate, Scale,Roate,Skew"];    
const styles = StyleSheet.create({
  container: {
    flex: 1,
    flexDirection: 'column',
    justifyContent: 'center',
    alignItems: 'center',
  },
  view : {
    alignItems: 'center',
    justifyContent: 'center',
     width: 500, 
     height: 500,
     borderWidth:5,
    borderColor:'red',
    backgroundColor:'lightblue'
  }

}
);
var Componet ="View";

var count =0;
var list=["View","Image","Text"];
const SimpleApp = React.Node = () => {

  if(test=="Translate") {
    var transformProps = translateProps;
    var TestCaseCount = translateProps.length;
    var TestCasesName = translateTestCases;
  }
  if(test=="Scale") {

    var transformProps = scaleProps;
    var TestCaseCount = scaleProps.length;
    var TestCasesName = scaleTestCases;
  }
  if(test=="Rotate") {
    var transformProps = rotateProps;
    var TestCaseCount = rotateProps.length;
    var TestCasesName = rotateTestCases;
  }
  if(test=="Skew") {
    var transformProps = skewProps;
    var TestCaseCount = skewProps.length;
    var TestCasesName = skewTestCases;
  }
  if(test=="Combined") {
    var transformProps = combinedProps;
    var TestCaseCount = combinedProps.length;
    var TestCasesName = combinedTestCases;
  }
const [index, setindex] = useState(true);

const timer = setTimeout(()=>{
      if(index+1 == TestCaseCount) Componet=list[count];
      setindex((index+1)%TestCaseCount);
      }, TestCaseTimeOut)

  if(Componet=='View') {
    count=1;
    var name = test + " ["+Componet+ "] : " + TestCasesName[index];
    return (
      <View style={styles.container}>
      <Text style={{ textAlign: 'center'}}>{name}</Text>
     <View style={styles.view}>
        <View style={{    width: 200, 
     height: 200,
     borderWidth:30,
     borderRightColor:'#2196c4',
     borderLeftColor:'#2196c4',
     borderTopColor:'#51b8e1',
     borderBottomColor:'#51b8e1',
     backgroundColor:'#1a7599',
     transform:transformProps[index]}}>
   </View>
   </View>
      </View>

    );
  }
  else if(Componet=='Image') {
    count=2;
    var name = test + " ["+Componet+ "] : " + TestCasesName[index];
    return (
      <View style={styles.container}>
       <Text style={{ textAlign: 'center'}}>{name}</Text>
       <View style={styles.view}>
        <Image
         source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
         style= {{     width: 200, 
     height: 200,
     borderWidth:5,
     borderColor:'red',
   transform:transformProps[index]}}
      />
      </View>
      </View>
    );
  }
  else if(Componet=='Text') {
    count=0;
    var name = test + " ["+Componet+ "] : " + TestCasesName[index];
    return (
      <View style={styles.container}>
      <Text style={{ textAlign: 'center'}}>{name}</Text>
      <View style={styles.view}>
      <Text style={{    fontSize: 36,
    fontWeight: 'bold',
    color: 'green',
    textAlign: 'center',transform:transformProps[index]}}>RN Skia</Text>
      </View>
      </View>
    );
  }
  else {
    return (
      <View style={styles.container}>
      </View>
    );
  }
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleApp);