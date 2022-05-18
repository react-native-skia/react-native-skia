/**
 * goto server directory.
 * $ cd <path of source code>//src/packages/react-native-skia/components/image/server
 *
 * start 6 server for no-cache, no-store, max-age=0, max-age is greter than 30min,
 * max-age less than 30 min and max-age=30 min.
 *
 * start server exmple:
 * $ python3 <ServerName>.py
 *
 */
 
 import * as React from 'react';
 import type {Node} from 'react';
 import {
         AppRegistry,
         Image,
         Text,
         View,
 } from 'react-native';
 import { useState } from 'react';
 
 const SimpleViewApp = React.Node = () => {
 var useCases = 10;
 var timerValue = 10000;
 /*give your machin ip address to ipAddress varible*/
 var ipAddress = '10.0.2.15';

 const [UseCaseCount, toggleViewState] = useState(true);
 const timer = setTimeout(()=>{
            toggleViewState((UseCaseCount+1)%useCases);
        }, timerValue)

 const renderMainView = () => {
   if(UseCaseCount == 1)
   {
       return (
        <>
        <View
        style={{  justifyContent: 'center',
                       alignItems: 'center',}}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,
                         
            }}/>
            <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image</Text>
        </View>
        </View>
        </>)
   }
   else if(UseCaseCount ==2)
   {
       return (
        <>
        <View
        style={{  justifyContent: 'center',
                       alignItems: 'center',}}>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',  
            }}>
            <Image
                source={{uri: 'https://pngimg.com/uploads/lion/lion_PNG23269.png'}}
                style={{ width: 200,
                         height: 200,
               }}/>
            <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Network Image</Text>   
        </View>
        </View>
  </>)

 }
   else if(UseCaseCount ==3)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>

        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
            }}>
            <Image
                source={{uri: 'https://png.pngtree.com/png-clipart/20210801/original/pngtree-resolution-logo-720p-tags-png-image_6578799.jpg  ',}}
                style={{ width: 200,
                         height: 200,     
               }}/>
        </View>
    </View>
    <View 
      style={{
                  flex:0.5,
                  flexDirection:'row',
                  justifyContent: 'center',
                  alignItems: 'center',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image </Text> 
    </View>
  </>)
   }
    else if(UseCaseCount ==4)
   {
       return (
          <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri: 'http://'+ipAddress+':8001/gangster_PNG70.png'}}
                style={{ width: 200,
                         height: 200,      
               }}/>
        </View>
    </View>
    <View 
      style={{  justifyContent: 'center',
                alignItems: 'center',
                flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize:30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image no-cache </Text> 
    </View>
  </>)
   }
   else if(UseCaseCount ==5)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri: 'http://'+ipAddress+':8002/ninja_PNG10.png'}}
                style={{ width: 200,
                         height: 200,      
               }}/>
        </View>
    </View>
    <View 
      style={{ justifyContent: 'center',
               alignItems: 'center',
               flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image no-store </Text> 
    </View>
  </>)
   }
   else if(UseCaseCount ==6)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri: 'http://'+ipAddress+':8003/2022_year_PNG23.png'}}
                style={{ width: 200,
                         height: 200,       
               }}/>
        </View>
    </View>
    <View 
      style={{ justifyContent: 'center',
               alignItems: 'center',
               flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image max-age=0 </Text> 
    </View>
  </>)
   }
   else if(UseCaseCount ==7)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri:'http://'+ipAddress+':8004/football_PNG52796.png'}}
                style={{ width: 200,
                         height: 200,
               }}/>
        </View>
    </View>
    <View 
      style={{  justifyContent: 'center',
                alignItems: 'center',
                flex:0.5,
                flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image max-age less than 30 min </Text> 
    </View>
  </>)
   }
   else if(UseCaseCount ==8)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri: 'http://'+ipAddress+':8005/robot_PNG94.png'}}
                style={{ width: 200,
                         height: 200,      
               }}/>
        </View>
    </View>
    <View 
      style={{    justifyContent: 'center',
                  alignItems: 'center',
                  flex:0.5,
                  flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image max-age = 30 min </Text> 
    </View>
  </>)
   }
   else if(UseCaseCount ==9)
   {
       return (
         <>
      <View
          style={{ justifyContent: 'center',
                   alignItems: 'center',
                   flexDirection:'row',
          }}>
         <View
             style={{  justifyContent: 'center',
                       alignItems: 'center',
                       width: 300,
                       height: 300,
                       margin:10,
                       backgroundColor:'cornsilk',
         }}>
            <Image
                source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200, 
                         height: 200 ,       
            }}/>
        </View>
        <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     width: 300,
                     height: 300,
                     margin:10,
                     backgroundColor:'cornsilk',
                     
            }}>
            <Image
                source={{uri: 'http://'+ipAddress+':8006/github_PNG75.png'}}
                style={{ width: 200,
                         height: 200,    
               }}/>
        </View>
    </View>
    <View 
      style={{    justifyContent: 'center',
                  alignItems: 'center',
                  flexDirection:'row',
          }}>
      <Text
           style={{ color: '#fff', fontSize: 30,
                       textAlign: 'center', marginTop: 32,color:'black' }}>
          Local Image  + Network Image max-age greater than 30 min </Text> 
    </View>
  </>)
   }
       else
   {
       return (
       <>
         <View
            style={{
            justifyContent: 'center',
            alignItems: 'center',}}>
          <View
            style={{
            justifyContent: 'center',
            alignItems: 'center',
            borderWidth: 2,
            height: 500,
            width: 500,
            margin:10,
           }}>
            <Image
                style={{ width:300, height: 300, backgroundColor: 'gray', }}
               source={require('react-native/Libraries/NewAppScreen/components/logo.png')}/>
            <Text
                style={{ color: '#fff', fontSize: 46,
                textAlign: 'center', marginTop: 32,color:'black' }}>
                React Native loves Skia</Text>
          </View>
         </View>
       </>)
    }
  }
  return (
    renderMainView()
   );
 };
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);