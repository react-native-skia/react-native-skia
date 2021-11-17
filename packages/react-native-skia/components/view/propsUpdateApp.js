import * as React from 'react';
import { useState } from 'react';
import { View, AppRegistry, Text,Image,TouchableOpacity,TouchableHighlight } from 'react-native';

const SimpleViewApp = React.Node = () => {
   var useCases =5;
   var timerValue=3000;
   const [UseCaseCount, toggleViewState] = useState(true);

   const timer = setTimeout(()=>{
           toggleViewState((UseCaseCount+1)%useCases);
       }, timerValue)
 
     var subViewWidth=150;
     var subViewHeight=300;

const renderMainView = () => {
  /*UseCase 1:- Demonstrate Angled border andd opacity */
  if(UseCaseCount == 1)
  {
	    return (<View
             style={{ flex: 1,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
	       opacity: 0.2,
	       borderWidth: 2,
               backgroundColor: 'orange',
	       pointEvents: 'auto',
	       overflow: 'hidden',
	       zIndex: 1,
             }}>
	<View>
          <Image
            style={{
              resizeMode: "cover",
              height: 100,
              width: 200,
	      tintColor: 'green'
            }}
            source={require("react-native/Libraries/NewAppScreen/components/logo.png")}
          />
        </View>
      </View>)
  }
  /*UseCase 2:- Demonstrate rounded border width  borderRadius & borderWidth Props*/
  else if(UseCaseCount ==2)
  {
      return (<View
             style={{ flex: 1,
                padding: 30,
                flexDirection: 'row',
                justifyContent: 'center',
                alignItems: 'center',
	        opacity: 0.5,	     
                backgroundColor: 'red',
                foregroundColor: 'green',
	        pointEvents: 'none',
	        overflow: 'visible',
	        zIndex: 2,
             }}>
        <View>
          <Image
            style={{
               resizeMode: "contain",
               height: 100,
	       tintColor: 'blue',
               width: 200,
            }}
             source={require("react-native/Libraries/NewAppScreen/components/logo.png")}
          />
        </View>
     </View>)
  }
  /*UseCase 3:- Demonstrate complex border*/
  else if(UseCaseCount ==3)
  {
      return (<View
             style={{flex: 1,
                padding: 30,
                flexDirection: 'row',
                justifyContent: 'center',
                alignItems: 'center',
                backgroundColor:'white',
	        opacity: 0.5,	     
                foregroundColor: 'red',
	        backfaceVisibility: 'hidden',
	        pointEvents: 'none',			     
	        overflow: 'hidden',
             }}>
      <View>
        <Image
          style={{
            resizeMode: "stretch",
            height: 100,
            width: 200,
	    tintColor: 'red'
          }}
           source={require("react-native/Libraries/NewAppScreen/components/logo.png")}
         />
      </View>
     </View>)
  }
/*UseCase 4:- Demonstrate Angled border*/
  else if(UseCaseCount ==4)
  {
      return (<View
             style={{flex: 1,
               padding: 30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
	       opacity: 0.3,	     
               backgroundColor: 'cyan',
               foregroundColor: 'blue',
	       pointEvents: 'box-none',			     
             }}>
        <View
          style={{
	     width: subViewWidth,
             height: subViewHeight,
             backgroundColor: 'red',
             shadowRadius: 0.3,
             shadowOpacity: 0.9,
             shadowColor: 'gray',
             shadowOffset: {
                           width: 80,
                           height: 80
            },
          }}>
        </View>
      </View>)
  }
   /*UseCase:- simple view for view & image component*/
  else 
  {
      return (<View
             style={{flex: 1,
               padding: 30,
               flexDirection: 'row',
               justifyContent: 'center',
               alignItems: 'center',
               backgroundColor: 'gray',
	        opacity: 0.6,	     
             }}>
	  <View
	    style={{
	       width: subViewWidth,
               height: subViewHeight,
               backgroundColor: 'orange',
               shadowRadius: 0.7,
               shadowColor: 'red',
               shadowOpacity: 0.1,
               transform: [{scale: 2}],
               shadowOffset: {
                             width: 100,
                             height: 100
              },
          }}>
      </View>
    </View>)
   }

 }

 return (
   renderMainView()
  );
};
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
