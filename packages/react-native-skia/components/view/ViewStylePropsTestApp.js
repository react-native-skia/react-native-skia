import * as React from 'react';
import { useState } from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
   var useCases =5;
   var timerValue=10000;
   const [UseCaseCount, toggleViewState] = useState(true);

   const timer = setTimeout(()=>{
           toggleViewState((UseCaseCount+1)%useCases);
       }, timerValue)
     
     var baseOpacity=0.8;
     var baseRadius=30;
     var baseWidth=20;
     var bgColor='cornflowerblue';
     var borderColor='green';
     var mainViewbgColor='pink';
     var mainViewborderColor='yellow';
     var subViewWidth=150;
     var subViewHeight=300;
   const renderImage = (opacity) => {
       return <Image
                    style={{width:250,height:250,backgroundColor: 'purple',borderWidth: baseWidth,borderColor:'darkcyan',
                            borderRadius:70,opacity:opacity,borderStyle:'solid',margin: 20,}}
                     source={require('react-native/Libraries/NewAppScreen/components/logo.png')}
                   />
       
   }
  const opacityTest=(opacity)=>{
           return <View
            style={{ width:subViewWidth,
               height : subViewHeight,
               backgroundColor: bgColor,
               borderWidth:baseWidth,
               borderColor:borderColor,
               opacity:opacity,
               margin: 20,
<<<<<<< HEAD
               borderRadius:baseRadius,
=======
	       borderRadius:baseRadius,
>>>>>>> Shadow Props supported on ReactNativeSkia and RSKDrawUtils refactored (#13)
           }}>
           </View> 
}
const roundedBorderTest =(width,radius)=>{
           return <View
            style={{ width:subViewWidth,
               height : subViewHeight,
               backgroundColor: bgColor,
               borderWidth:width,
               borderColor:borderColor,
<<<<<<< HEAD
              opacity:baseOpacity,
=======
               opacity:baseOpacity,
>>>>>>> Shadow Props supported on ReactNativeSkia and RSKDrawUtils refactored (#13)
               margin: 20,
               borderRadius:radius,
           }}>
           </View> 
}
const complexRoundedBorderTest =(radius,color,width)=>{
           return <View
            style={{ width:subViewWidth,
               height : subViewHeight,
               backgroundColor: bgColor,
               borderWidth:baseWidth,
               borderColor:borderColor,
               opacity:baseOpacity*10,
               margin: 20,
               borderTopWidth: width*2, 
               borderRightWidth:width,
               borderLeftWidth:width*2,
               borderRadius:radius,
               borderLeftColor:color,
               borderRightColor:color,
               borderBottomColor:borderColor,
               borderTopColor:borderColor,
<<<<<<< HEAD

=======
>>>>>>> Shadow Props supported on ReactNativeSkia and RSKDrawUtils refactored (#13)
              shadowOffset: {
                 width: subViewWidth/2,
                 height:subViewHeight/2
              },
              shadowRadius:20,
	      shadowColor:'red',
	      shadowOpacity: 1,

           }}>
           </View> 
}

const renderMainView = () => {
  /*UseCase 1:- Demonstrate Angled border andd opacity */
  if(UseCaseCount == 1)
  {
	    return (<View
             style={{ flex: 1,
                padding:30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor:mainViewbgColor,
               borderRightWidth:baseWidth*2,
               borderLeftWidth:baseWidth*3,
               borderBottomWidth:baseWidth/2,
               borderWidth:baseWidth,
               borderLeftColor:'blue',
               borderRightColor:'yellow',
               borderTopColor:'orange',
               opacity:baseOpacity*10,
             }}>
            
           {opacityTest(baseOpacity)}
           {opacityTest(baseOpacity/2)}
           {opacityTest(baseOpacity*0)}
           {opacityTest(baseOpacity*8)}

           </View>)
  }
  /*UseCase 2:- Demonstrate rounded border width  borderRadius & borderWidth Props*/
  else if(UseCaseCount ==2)
  {
      return (<View
             style={{ flex: 1,
               padding:30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               borderRadius:baseRadius,
               borderColor:mainViewborderColor,
               borderWidth:baseWidth,
               opacity:baseOpacity*10,
             }}>
            
           {roundedBorderTest(baseWidth*2,baseRadius*2)}
           {roundedBorderTest(baseWidth*0,baseRadius*0)}
           {roundedBorderTest(baseWidth,baseRadius*2)}
           {roundedBorderTest(baseWidth,baseRadius/2)}

           </View>)
  }
  /*UseCase 3:- Demonstrate complex border*/
  else if(UseCaseCount ==3)
  {
      return (<View
             style={{ flex: 1,
                padding:30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor:mainViewbgColor,
               borderTopWidth: baseWidth*2, 
               borderRightWidth:baseWidth*3,
               borderLeftWidth:baseWidth*2,
               borderBottomWidth:baseWidth,
               borderRadius:baseRadius*2,
               borderLeftColor:'firebrick',
               borderRightColor:'green',
               borderBottomColor:'blue',
               borderTopColor:'orange',
               opacity:baseOpacity*10,
             }}>
            
           {complexRoundedBorderTest(baseRadius/2,'yellow',baseWidth/2)}
           {complexRoundedBorderTest(baseRadius*2,'blue',baseWidth)}
           {complexRoundedBorderTest(baseRadius,'green',baseWidth)}
           </View>)
  }
/*UseCase 4:- Demonstrate Angled border*/
  else if(UseCaseCount ==4)
  {
      return (<View
             style={{ flex: 1,
                padding:30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor:mainViewbgColor,
               borderWidth: baseWidth, 
               borderColor:mainViewborderColor,
               borderTopColor:'orange',
               borderRightColor:'firebrick',
               borderBottomColor:'green',
               borderLeftWidth:baseWidth*2,
               borderTopWidth:baseWidth*3,
               opacity:baseOpacity*10,
             }}>
            
<<<<<<< HEAD
<<<<<<< HEAD
           {renderImage(baseOpacity)}
           {renderImage(baseOpacity/2)}
=======
           {renderImage(baseOpacity*10)}
           {renderImage(baseOpacity*5)}
>>>>>>> Added support on Native side to handle View Style Props (#3)
=======
           {renderImage(baseOpacity)}
           {renderImage(baseOpacity/2)}
>>>>>>> Shadow Props supported on ReactNativeSkia and RSKDrawUtils refactored (#13)

           </View>)
  }
   /*UseCase:- simple view for view & image component*/
  else 
  {
      return (<View
             style={{ flex: 1,
                padding:30,
                flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor:mainViewbgColor,
               borderWidth:baseWidth, 
               borderColor:mainViewborderColor,
               opacity:baseOpacity*10,
             }}>
<<<<<<< HEAD
<<<<<<< HEAD
	   {renderImage(baseOpacity/2)}
           {renderImage(baseOpacity)}
=======
           {renderImage(baseOpacity*10)}
           {renderImage(baseOpacity*5)}
>>>>>>> Added support on Native side to handle View Style Props (#3)
=======
           {renderImage(baseOpacity/2)}
           {renderImage(baseOpacity)}
>>>>>>> Shadow Props supported on ReactNativeSkia and RSKDrawUtils refactored (#13)
           </View>)
   }

 }
 return (
   renderMainView()
  );
};

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
