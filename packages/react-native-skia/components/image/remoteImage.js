/*
https://pngimg.com/uploads/lion/lion_PNG23269.png
https://pngimg.com/uploads/tiger/tiger_PNG23244.png
https://pngimg.com/uploads/fish/fish_PNG25146.png
https://pngimg.com/uploads/arctic_fox/arctic_fox_PNG41376.png
https://pngimg.com/uploads/panda/panda_PNG10.png
https://pngimg.com/uploads/gorilla/gorilla_PNG18708.png
https://pngimg.com/uploads/mango/mango_PNG9173.png
https://pngimg.com/uploads/birds/birds_PNG95.png
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
var useCases =5;
var timerValue=10000;

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
            style={{
              flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/lion/lion_PNG23269.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'ivory',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/tiger/tiger_PNG23244.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'yellow',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/paper_plane/paper_plane_PNG54.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'mintcream',
               }}/>
            </View>

          </View>
          <View
            style={{
                flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/arctic_fox/arctic_fox_PNG41376.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'blue',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Text
                style={{ color: '#fff',
                         fontSize: 46,
                         textAlign: 'center',
                         color: 'black',
                         marginTop: 16 }}>
                Network Image
              </Text>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/panda/panda_PNG10.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'green',
                }}/>
            </View>
          </View>
        </>
      )
    }
  else if(UseCaseCount ==2)
     {
      return (
        <>
          <View
            style={{
                flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
               source={require('./disney-1-logo-png-transparent.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'ivory',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={require('./rolls_royce_PNG36.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'yellow',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={require('./index.jpeg')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'mintcream',
               }}/>
            </View>
          </View>
          <View
            style={{
                flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={require('./raspberry_PNG5077.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'blue',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Text
                style={{ color: '#fff',
                         fontSize: 46,
                         textAlign: 'center',
                         color: 'black',
                         marginTop: 16 }}>
                Local Image
              </Text>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={require('./audi_PNG1767.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'green',
                }}/>
            </View>
          </View>
        </>
      )
    }
 else if(UseCaseCount ==3)
   {
       return (
        <>
          <View
            style={{
                flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                source={{uri: 'https://pngimg.com/uploads/beach/beach_PNG86.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'ivory',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
               <Text
                style={{ color: '#fff',
                         fontSize: 46,
                         textAlign: 'center',
                         color: 'black',
                         marginTop: 16 }}>
                Network Image
              </Text>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
             <Image
                source={{uri: 'https://pngimg.com/uploads/peacock/peacock_PNG19.png'}}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'yellow',
                }}/>
            </View>
          </View>
          <View
            style={{
                flexDirection:'row',
            }}>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
              <Image
                 source={require('./rolls_royce_PNG36.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'ivory',
                }}/>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
                <Text
                style={{ color: '#fff',
                         fontSize: 46,
                         textAlign: 'center',
                         color: 'black',
                         marginTop: 16 }}>
                Local Image
              </Text>
            </View>
            <View
              style={{ width: 250,
                       height: 250,
                       margin:10,
                       backgroundColor:'cornsilk',
              }}>
             <Image
                source={require('./audi_PNG1767.png')}
                style={{ width: 200,
                         height: 200,
                         backgroundColor:'yellow',
                }}/>
            </View>
          </View>
        </>
      )
   }
 else if(UseCaseCount ==4)
   { 
     return (
       <View
         style={{
                  justifyContent: 'center',
                  alignItems: 'center',
                  borderWidth: 2,
                  height: 500,
                  width: 800,
         }}>
         <Image
           style={{ width: 400,
                    height: 400,
                    backgroundColor: 'aquamarine', }}
           source={{uri: 'https://pngimg.com/uploads/lion/lion_PNG23269.png'}}/>
         <Text
           style={{ color: '#fff', fontSize: 46,color:'black',
                    textAlign: 'center', marginTop: 32 }}>
           Change network image in 10 sec ...
         </Text>
       </View>)
     }
    else
      {
        return (
          <View
            style={{ justifyContent: 'center',
                     alignItems: 'center',
                     borderWidth: 2,
                     height: 500,
                     width: 800,
            }}>
            <Image
              style={{ width: 400,
                       height: 400,
                       backgroundColor: 'aquamarine', }}
              source={{uri: 'https://pngimg.com/uploads/spider_man/spider_man_PNG84.png'}}/>
            <Text
                style={{ color: '#fff', fontSize: 46,
                textAlign: 'center', marginTop: 32,color:'black', }}>
                Repeat again all cases ....
            </Text>
          </View>)
      }
  }
  return (
    renderMainView()
  );
 };
 AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);