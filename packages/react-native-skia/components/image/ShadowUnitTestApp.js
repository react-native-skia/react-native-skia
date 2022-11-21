import React, { useState} from "react";
import { AppRegistry,StyleSheet,Dimensions} from "react-native";
import { View,Text,Image,TouchableHighlight} from "react-native";

const windowSize = Dimensions.get('window');

const TestPages = [
    {Title:"Show casing Frame types",Name:'View'},
    {Title:"Show casing Shadow types : On View",Name:'Shadow'},
    {Title:"Show casing Shadow types : On View With Transform",Name:'ShadowWithTransform'},
    {Title:"Show casing Shadow types : On JPEG Image",Name:'JPEGImageShadow'},
    {Title:"Show casing Shadow types : On PNG Image",Name:'PNGImageShadow'},
    {Title:"Show casing Shadow types : On Image With Resize Mode",Name:'ImageShadowWithResizeMode'},
];

var BorderTypes =[
    "BG + monChrome Border",
    "BG + PolyChrome Border",
    "No BG + Default Border",
    "Invisible BG + Border with Fully Transparent Alpha",
    "Dicreate Border with Varying width",
    "Dicreate Border with invisble sides width",
]

var FramesPerRow_FrameTypes = 4; // Max of only 2 rows in a page
const TestCase_FrameTypes = [
    {Title:"BG + monChrome Border",backgroundColor:'burlywood',borderColor:'red',AdditionDescription:''},
    {Title:"Default monoChrome Border",AdditionDescription:'BorderWidth : Non Zero, Border & BackGround Color:Not Specified'},
    {Title:"BG + PolyChrome Border",backgroundColor:'burlywood',borderBottomColor:'red',borderRightColor:'yellow',AdditionDescription:'Border Color not specified for Left & Top'},
    {Title:"BG+Border with transparent Alpha",borderColor:'#ff000000',AdditionDescription:'All sides has Transparent alpha'},

    {Title:"BG +InVisible Border",backgroundColor:'burlywood',borderColor:'red',borderWidth:0,AdditionDescription:'BorderWidth :0, Color:Valid'},
    {Title:"BG +InVisible Border",backgroundColor:'burlywood',borderColor:'#ff000000',AdditionDescription:'BorderWidth :valid & transparent Color'},
    {Title:"BG +Discrete Border",backgroundColor:'burlywood',borderRightWidth:0,borderLeftWidth:0,AdditionDescription:'Border width invalid for missing sides + Border Color Not specified'},
    {Title:"Discrete Border",borderRightWidth:15,borderBottomWidth:5,borderTopWidth:5,borderBottomColor:'red',borderRightColor:'yellow',AdditionDescription:'NO BG+ Border with varying width'},
];

var FramesPerRow = 3; 
const shadowProperty={
    shadowColor:'aqua',
    shadowOpacity:1,
    shadowOffset:{width:20, height:20}}

const TestCase_ShadowOnView = [
    {Title:"BG + Rounded Rect Border",backgroundColor:'burlywood',borderColor:'red',AdditionDescription:''},
    {Title:"No BG + Rect Border ",borderRadius : 0,borderColor:'red',AdditionDescription:'BorderWidth :Valid, BackGround Color:Not Specified'},
    {Title:"polyChrome Rect Border",borderBottomColor:'red',AdditionDescription:'NO BG + BorderWidth : Non Zero, Color:Not Specified'},
    {Title:"Discrete Border",borderBottomColor:'red',borderRightColor:'yellow',AdditionDescription:'NO BG + Borders with varying color & uniform width'},
    {Title:"Discrete Border",borderRightWidth:0,borderLeftWidth:0,AdditionDescription:'NO BG + Borders with varying/invisible width'},
    {Title:"InVisible BG+Border",borderColor:'#ff000000',AdditionDescription:'Background color : Not Specified+ BorderColor : has Alpha Zero'},
];

const TestCase_ShadowOnViewWithTransform = [
    {Title:"BG + Rect Border",backgroundColor:'burlywood',borderColor:'red',AdditionDescription:'No Transform'},
    {Title:"BG + Rect Border",transform:[{rotate: "45deg"}] ,backgroundColor:'burlywood',borderColor:'red',AdditionDescription:'Transform : Rotate'},
    {Title:"BG + Rect Border",transform:[{skewY:'45deg'}],backgroundColor:'burlywood',borderColor:'red',AdditionDescription:'Transform : Skew'},
    {Title:"BG + Rect Border",transform:[{scaleY:2}],backgroundColor:'burlywood',borderColor:'red',AdditionDescription:'Transform : Scale'},
];

var ImageSources = [
    require('./cake.png'),
    require('./1.jpg')
];
const TestCase_ImageShadow=[
    {Title:"BG + Rect Border ",backgroundColor:'burlywood',borderColor:'red',borderWidth:5,resizeMode:'cover',AdditionDescription:''},
    {Title:"No BG + Rect Border",borderColor:'red',resizeMode:'cover',borderWidth:5,AdditionDescription:''},
    {Title:"No BG + Dicrete Border ",borderWidth:5,borderBottomColor:'red',borderRightColor:'yellow',resizeMode:'cover',AdditionDescription:''},
    {Title:"No BG + Dicrete Border ",borderWidth:5,borderTopWidth:0,borderLeftWidth:0,borderColor:'blue',borderBottomColor:'red',borderRightColor:'yellow',resizeMode:'cover',AdditionDescription:''},
    {Title:"No BG + No Border ",borderWidth:0,resizeMode:'cover',AdditionDescription:''},
    {Title:"Fully transparent Shadow",borderWidth:5,shadowOpacity:0,resizeMode:'cover',AdditionDescription:''},
];

var resizemodes = [
    "center",
//    "cover",
    "contain",
//    "stretch",
    "repeat",
];

const TestCase_ImageShadowWithResize=[
    {Title:"ResizeMode:Center",resizeMode:'center',shadowOffset:{width:40, height:40},source:ImageSources[0]},
    {Title:"ResizeMode:Contain",resizeMode:'contain',borderColor:'red',borderWidth:5,source:ImageSources[0]},
    {Title:"ResizeMode:Repeat",resizeMode:'repeat',borderColor:'red',borderWidth:5,source:ImageSources[0]},
    {Title:"ResizeMode:Center",resizeMode:'center',borderColor:'red',borderWidth:5,source:ImageSources[1]},
    {Title:"ResizeMode:Contain",resizeMode:'contain',borderColor:'red',borderWidth:5,source:ImageSources[1]},
    {Title:"ResizeMode:Repeat",resizeMode:'repeat',borderWidth:5,source:ImageSources[1]},
];

const getFrames = (n,count,testCase,testCaseTitle,imageSource) => {
   var arr = [];
   var stylesProps=[];styles.ContainerStyle;

   if(testCaseTitle.search("Image") != -1){
     stylesProps = styles.ImageContentStyle;
   } else {
     stylesProps = styles.contentStyle;
   }
   if(testCaseTitle.search("Shadow") != -1){
     stylesProps = Object.assign({},stylesProps,shadowProperty);
   }
   for (var i=n; i<(n+count); i++){
        var props=[];
       props = Object.assign({}, stylesProps,testCase[i]);
       console.log("Test Case :",props);

      if(testCaseTitle.search("Image") != -1) {
        if(testCaseTitle.search("Resize") != -1) {
            imageSource=props.source;
        }
        arr.push(
            <View style={styles.ImageContainerStyle}>
                <Text style ={styles.TextStyle}>{props.Title}</Text>
                <Image
                  style={props}
                  source={imageSource}
                  onLoad={()=>console.log("Image Loaded Succesfully")}
                  onError={()=>console.log("Image Loading Failed")}
                />
            </View>

        );
      } else {
          arr.push(  <View style={props}>
                        {getFramesDescription(testCase[i])}
                    </View>);
      }
   }
   return arr;
}

const getFramesDescription = (FrameProps) => {
   var arr = [];
   var headerStyle = Object.assign({}, styles.TextStyle,
                                      {color:'blue',
                                       fontWeight:'bold'});
    return   (     <Text>
                   <Text style ={headerStyle}>{FrameProps.Title}{'\n'}{'\n'}</Text>
                   <Text style ={styles.TextStyle}>{FrameProps.AdditionDescription}{'\n'}</Text>
                  </Text>
             );
}

const getTestCases=(testCaseTitle)=>{

    var TestCases =[];
    var framesPerRow=FramesPerRow;
    var imageSource;

    switch(testCaseTitle){
        case 'View':
            TestCases=TestCase_FrameTypes;
            framesPerRow=FramesPerRow_FrameTypes;
        break;
        case 'Shadow':
            TestCases=TestCase_ShadowOnView;
        break;
        case 'ShadowWithTransform':
            framesPerRow=FramesPerRow_FrameTypes;
            TestCases=TestCase_ShadowOnViewWithTransform;
        break;
        case 'JPEGImageShadow':
            imageSource=ImageSources[1];
            TestCases=TestCase_ImageShadow;
        break;
        case 'PNGImageShadow':
            imageSource=ImageSources[0];
            TestCases=TestCase_ImageShadow;
        break;
        case 'ImageShadowWithResizeMode':
            TestCases=TestCase_ImageShadowWithResize;
        break;
    }

    var arr = [];
    if(framesPerRow >= TestCases.length) {
        arr.push(<View style={styles.ContainerStyle}>
                    {getFrames(0,TestCases.length,TestCases,testCaseTitle,imageSource)}
                  </View>
                );
    } else {
        arr.push (
            <View style={styles.ContainerStyle}>
                {getFrames(0,framesPerRow,TestCases,testCaseTitle,imageSource)}
            </View>
        )
        if( TestCase_FrameTypes.length > framesPerRow) {
            arr.push (
                 <View style={styles.ContainerStyle}>
                     {getFrames(framesPerRow,TestCases.length-framesPerRow,TestCases,testCaseTitle,imageSource)}
                 </View>
            )
        }
    }
    return arr;
}

const UnitTestApp = () => {

    let [TestCaseCount, setTestCase] = useState(0);

    const getButtons =()=>{
        var arr=[];
        if(TestCaseCount<(TestPages.length-1)) {
            arr.push(<TouchableHighlight underlayColor='#61dafb'
                      style={styles.button}
                      onPress={() => {
                          setTestCase(TestCaseCount +1 )
                          console.log("calling next")} 
                        }
                      >
                      <Text>Next</Text>
                    </TouchableHighlight>);
        }
        if(TestCaseCount > 0) {
            arr.push(<TouchableHighlight underlayColor='#61dafb'
                      style={styles.button}
                      onPress={() => {
                            setTestCase(TestCaseCount -1)
                            console.log("calling Previous")} 
                        }
                      >
                      <Text>Previous</Text>
                    </TouchableHighlight>);
        }
        return arr;
    }
    return (
        <View style={styles.PageStyle}>
            <Text style={styles.HeaderTextStyle}>{TestPages[TestCaseCount].Title}</Text>
                {getTestCases(TestPages[TestCaseCount].Name)}
            <View style={styles.ButtonView}>
                {getButtons()}
            </View>
        </View>
    )
}

const styles = StyleSheet.create({
    PageStyle: {
        marginTop: '2%',
        marginLeft:'2%',
       alignItems : 'center',
       justifyContent : 'space-evenly',
       flexDirection:'column',
       borderWidth: 3,
       width : '95%',
       height : '95%',
       backgroundColor:'#5f9ea0',//cadetblue
    },
    ContainerStyle: {
       marginLeft:'1%',
       marginLeft:'2%',
       justifyContent : 'space-between',
       flexDirection:'row',
       width : '95%',
       padding:'1%',
       height : windowSize.height*.40,
       borderWidth: 3,
       borderRadius : 10,
       backgroundColor:'transparent',
    },
     ImageContainerStyle: {
        marginLeft:'1%',
        marginLeft:'1%',
        justifyContent : 'space-between',
        flexDirection:'column',
        width : '20%',
        height :'95%',
        padding:'1%',
     },
     ImageContentStyle: {
         marginTop:'1%',
         marginLeft:'1%',
         height :'95%',
         width : '95%',
         alignItems : 'center',
         justifyContent : 'center',
    },
    contentStyle: {
       marginTop:'1%',
       marginLeft:'1%',
       height :'95%',
       width : '20%',
       alignItems : 'center',
       justifyContent : 'center',
       borderWidth:10,
       borderRadius : 20
    },
    ButtonView: {
       marginRight:'2%',
       marginLeft: '2%',
       width : '95%',
       alignItems : 'flex-start',
       justifyContent : 'space-between',
       flexDirection:'row',
    },
    button: {
      alignItems: "center",
      backgroundColor: "#DDDDDD",
      padding: 10,
      width: windowSize.width*.1
    },
    HeaderTextStyle: {
       fontSize : windowSize.width/50,
       fontWeight:'bold'
    },
    TextStyle: {
        textAlign:'center'
    },


});
AppRegistry.registerComponent('SimpleViewApp', () => UnitTestApp);

export default UnitTestApp

