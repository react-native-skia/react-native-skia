import React, {Component} from 'react';
import {Svg, Circle, G, Line, Rect, Ellipse,Use,Path,LinearGradient,TSpan,Stop,Defs,Polygon,Polyline} from 'react-native-svg';
import { AppRegistry, StyleSheet, View,Text } from "react-native";

class CircleIcons extends Component {
  static title = 'Rect';
  render() {
    return (
      <View>
        <Text style={{fontSize:'20',textAlign: 'center'}}>ION ICONS</Text>
        <View style={styles.top}>
          <View style={styles.container}>
            <View style={styles.innerContainer}>
              <Text style={{fontSize:'12'}}> menu More</Text>
                <Svg xmlns="http://www.w3.org/2000/Svg" width={16} height={4}  >
                  <G fill="blue" fillRule="evenodd">
                    <Circle cx={2} cy={2} r={2} />
                     <Circle cx={8} cy={2} r={2} />
                     <Circle cx={14} cy={2} r={2} />
                 </G>
              </Svg>
             </View>
                <View style={styles.innerContainer}>
                  <Text style={{fontSize:'12'}}> menu Focussed</Text>
                    <Svg xmlns="http://www.w3.org/2000/svg" width={18} height={18} >
                       <G fill="none" fillRule="evenodd">
                           <Path
                            fill="blue"
                            d="M10.801 10.632a.91.91 0 0 1 1.284 0l4.707 4.707a.906.906 0 0 1 0 1.283l-.064.063a.908.908 0 0 1-1.283.001l-4.708-4.707a.906.906 0 0 1 .001-1.283l.063-.064z"
                           />
                           <Circle
                                  cx={6.882}
                                  cy={6.882}
                                  r={6.132}
                                  stroke="blue"
                                  strokeWidth={1.5}
                            />
                         </G>
                       </Svg>
                   </View>
                  <View style={styles.innerContainer}>
                    <Text style={{fontSize:'12'}}> menu Search</Text>
                      <Svg xmlns="http://www.w3.org/2000/svg" width={24} height={25} >
                        <G fill="none" fillRule="evenodd">
                            <Path
                                  fill="blue"
                                  d="M15.51 16.664a.821.821 0 0 1 1.193.008l6.429 6.755c.33.347.335.904.004 1.25l.187-.196a.82.82 0 0 1-1.192-.007l-6.43-6.755a.912.912 0 0 1-.004-1.25l-.186.195z"
                            />
                        <Ellipse
                        cx={9.5}
                        cy={9.896}
                        stroke="blue"
                        strokeWidth={1.125}
                        rx={8.938}
                        ry={9.333}
                      />
                    </G>
                  </Svg>
                  </View>
                     <View style={styles.innerContainer}>
                       <Text style={{fontSize:'12'}}> menu News</Text>
                     <Svg xmlns="http://www.w3.org/2000/svg" width={20} height={18} >
                       <G fill="none" fillRule="evenodd" transform="translate(1 1)">
                         <Path
                           fill="#202020"
                           stroke="#35FFA7"
                           d="M18.5 2.5V15c0 .414-.168.79-.44 1.06a1.49 1.49 0 0 1-1.06.44h-2a1.49 1.49 0 0 1-1.06-.44A1.49 1.49 0 0 1 13.5 15V2.5h5z"
                         />
                         <Rect
                           width={17}
                           height={17}
                           x={-0.5}
                           y={-0.5}
                           fill="#202020"
                           stroke="#35FFA7"
                           rx={1}
                         />
                         <Path
                           fill="#35FFA7"
                           d="M2 2h5v5H2zm7 1h5v1H9zm0 2h5v1H9zm-7 5h12v1H2zm0 2h12v1H2z"
                         />
                       </G>
                     </Svg>
                       </View>
                 </View>

                 <View style={styles.container}>
                    <View style={styles.innerContainer}>
                      <Text style={{fontSize:'12'}}> ICON Start Over</Text>
                       <Svg xmlns="http://www.w3.org/2000/svg" width={20} height={18} >
                         <G fill="black" fillRule="evenodd" transform="matrix(-1 0 0 1 19.947 0)">
                           <Path d="M4.547 2.282C4.547.62 5.671-.003 7.07.898L17.3 7.485c1.394.897 1.375 2.301-.068 3.153L7.139 16.595c-1.431.844-2.592.188-2.592-1.479V2.282z" />
                           <Rect
                             width={2.274}
                             height={17.432}
                             rx={1.137}
                             transform="rotate(180 1.137 8.716)"
                           />
                         </G>
                       </Svg>
                  </View>
                <View style={styles.innerContainer}>
              <Text style={{fontSize:'12'}}> Icon lock</Text>
            <Svg xmlns="http://www.w3.org/2000/svg" width={49} height={70} >
              <G fill="black" fillRule="evenodd" opacity={0.8}>
                <Path d="M26.429 48.785v-5.112c0-1.241-.96-2.244-2.143-2.244-1.183 0-2.143 1.003-2.143 2.244v5.112A4.517 4.517 0 0 0 20 52.653c0 2.478 1.92 4.49 4.286 4.49 2.365 0 4.285-2.012 4.285-4.49a4.517 4.517 0 0 0-2.142-3.868" />
                <Path d="M44.156 63.438a2.2 2.2 0 0 1-2.208 2.187H6.623a2.2 2.2 0 0 1-2.207-2.188v-26.25A2.2 2.2 0 0 1 6.623 35h35.325a2.2 2.2 0 0 1 2.208 2.188v26.25zm-1.172-32.732h-.9l.003-.081V17.5c0-9.65-7.923-17.5-17.662-17.5C14.687 0 6.763 7.85 6.763 17.5v13.125l7.66.08a6.693 6.693 0 0 0-1.037-.08h-2.208V17.5c0-7.238 5.941-13.125 13.247-13.125S37.672 10.262 37.672 17.5v13.125c0 .027.003.054.004.08H5.587C2.426 31.2 0 33.92 0 37.189v26.25C0 67.055 2.972 70 6.623 70h35.325c3.652 0 6.623-2.944 6.623-6.563v-26.25c0-3.268-2.425-5.987-5.587-6.481z" />
              </G>
            </Svg>
            </View>
            <View style={styles.innerContainer}>
               <Text style={{fontSize:'12'}}> Icon Play Camera</Text>
                  <Svg xmlns="http://www.w3.org/2000/svg" width={23} height={15} >
                    <Path
                      fill="black"
                      fillRule="evenodd"
                      d="M16.111 0c.954 0 1.728.774 1.728 1.728v2.588c2.345-1.046 4.496-1.92 4.741-1.806.56.262.56 9.721 0 9.98-.245.113-2.396-.77-4.74-1.828v2.61A1.73 1.73 0 0 1 16.111 15H1.728A1.728 1.728 0 0 1 0 13.272V1.728C0 .774.774 0 1.728 0h14.383zM5.951 4.52l-.005.087v5.773c0 .36.24.536.55.412l.078-.037 5.052-2.762c.313-.17.348-.455.09-.657l-.078-.053-5.075-3.026c-.319-.19-.575-.07-.612.262z"
                    />
               </Svg>
               </View>
                  <View style={styles.innerContainer}>
                     <Text style={{fontSize:'12'}}> Icon Share</Text>
                   <Svg xmlns="http://www.w3.org/2000/svg" width={16} height={18} >
                     <G fill="black">
                       <Path d="M15.273 3.187c0 1.347-1.058 2.438-2.364 2.438-1.305 0-2.364-1.091-2.364-2.438 0-1.346 1.059-2.437 2.364-2.437 1.306 0 2.364 1.091 2.364 2.437z" />
                       <Path d="M13.09 6c-1.604 0-2.908-1.346-2.908-3s1.304-3 2.909-3C14.695 0 16 1.346 16 3s-1.305 3-2.91 3zm0-4.875c-1.002 0-1.817.842-1.817 1.875s.815 1.875 1.818 1.875c1.003 0 1.818-.842 1.818-1.875s-.815-1.875-1.818-1.875zm2.183 13.688c0 1.346-1.058 2.437-2.364 2.437-1.305 0-2.364-1.091-2.364-2.437 0-1.347 1.059-2.438 2.364-2.438 1.306 0 2.364 1.091 2.364 2.438z" />
                       <Path d="M13.09 18c-1.604 0-2.908-1.346-2.908-3s1.304-3 2.909-3C14.695 12 16 13.346 16 15s-1.305 3-2.91 3zm0-4.875c-1.002 0-1.817.842-1.817 1.875s.815 1.875 1.818 1.875c1.003 0 1.818-.842 1.818-1.875s-.815-1.875-1.818-1.875zm-8-3.937c0 1.346-1.057 2.437-2.363 2.437-1.305 0-2.363-1.091-2.363-2.438 0-1.346 1.058-2.437 2.363-2.437 1.306 0 2.364 1.091 2.364 2.438z" />
                       <Path d="M2.909 12C1.305 12 0 10.655 0 9s1.305-3 2.909-3c1.604 0 2.91 1.345 2.91 3s-1.306 3-2.91 3zm0-4.875c-1.003 0-1.818.841-1.818 1.875s.815 1.875 1.818 1.875c1.003 0 1.818-.841 1.818-1.875S3.912 7.125 2.91 7.125z" />
                       <Path d="M4.379 8.625a.737.737 0 0 1-.646-.39.79.79 0 0 1 .278-1.052l6.88-4.081a.726.726 0 0 1 1.012.288.791.791 0 0 1-.277 1.053l-6.881 4.08a.704.704 0 0 1-.366.102zM11.258 15a.716.716 0 0 1-.367-.101l-6.88-4.081a.79.79 0 0 1-.278-1.053.726.726 0 0 1 1.012-.288l6.88 4.081c.356.21.48.682.278 1.052a.737.737 0 0 1-.645.39z" />
                     </G>
                      </Svg>
                    </View>
                </View>
              <View style={styles.container}>

               <View style={styles.innerContainer}>
               <Text style={{fontSize:'12'}}> Icon Remind</Text>
               <Svg xmlns="http://www.w3.org/2000/svg" width={15} height={18} >
                 <Path
                   fill="blue"
                   fillRule="evenodd"
                   d="M9.265 16.2c0 .994-.79 1.8-1.765 1.8-.974 0-1.765-.806-1.765-1.8h3.53zM7.507 0c.686 0 1.25.534 1.314 1.217l.006.13v.617c2.491.6 4.354 2.859 4.405 5.57h.002l.001 5.066L15 14.4v.9H0v-.9l1.779-1.78V7.535l.001-.001.004-.134c.11-2.652 1.952-4.847 4.403-5.436v-.618C6.187.603 6.778 0 7.507 0z"
                 />
               </Svg>
               </View>
               <View style={styles.innerContainer}>
               <Text style={{fontSize:'12'}}> Icon Single Recording</Text>
               <Svg xmlns="http://www.w3.org/2000/svg" width={16} height={12} >
                 <Path
                   fill="black"
                   fillRule="evenodd"
                   d="M9.692 7.151c.237.285.58.427 1.028.427.475 0 .94-.16 1.393-.478l.446 1.018c-.214.183-.49.329-.825.438-.337.11-.685.165-1.045.165-.574 0-1.073-.118-1.498-.353a2.39 2.39 0 0 1-.978-.994c-.227-.428-.34-.924-.34-1.487 0-.564.113-1.058.34-1.484.227-.425.553-.755.978-.99.425-.235.924-.352 1.498-.352.36 0 .708.054 1.045.164.336.11.611.256.825.438l-.446 1.018c-.454-.318-.918-.477-1.393-.477-.448 0-.791.14-1.028.422-.238.282-.356.702-.356 1.26 0 .56.118.98.356 1.265m-5.18 0c.237.285.58.427 1.029.427.474 0 .939-.16 1.393-.478l.446 1.018c-.214.183-.49.329-.826.438-.336.11-.684.165-1.044.165-.574 0-1.073-.118-1.499-.353a2.396 2.396 0 0 1-.978-.994c-.226-.428-.34-.924-.34-1.487 0-.564.114-1.058.34-1.484.227-.425.553-.755.978-.99.426-.235.925-.352 1.499-.352.36 0 .708.054 1.044.164.337.11.612.256.826.438l-.446 1.018c-.454-.318-.919-.477-1.393-.477-.449 0-.792.14-1.029.422-.237.282-.356.702-.356 1.26 0 .56.119.98.356 1.265M12.894.833H2.544A2.05 2.05 0 0 0 .5 2.878v6.357a2.05 2.05 0 0 0 2.043 2.044h10.351a2.05 2.05 0 0 0 2.043-2.044V2.878A2.05 2.05 0 0 0 12.894.833"
                 />
               </Svg>
               </View>
               <View style={styles.innerContainer}>
               <Text style={{fontSize:'12'}}> Icon Warning</Text>
               <Svg xmlns="http://www.w3.org/2000/svg" width={70} height={60} >
                 <Path
                   d="M69.384 53.443 38.673 2.09A4.285 4.285 0 0 0 35 0a4.281 4.281 0 0 0-3.673 2.091L.616 53.443a4.357 4.357 0 0 0-.062 4.36A4.285 4.285 0 0 0 4.289 60h61.422c1.547 0 2.974-.84 3.735-2.198a4.357 4.357 0 0 0-.062-4.36zm-34.361-35.68c1.764 0 3.256 1.003 3.256 2.781 0 5.427-.633 13.224-.633 18.65 0 1.413-1.538 2.006-2.623 2.006-1.447 0-2.668-.593-2.668-2.006 0-5.426-.633-13.223-.633-18.65 0-1.778 1.447-2.781 3.3-2.781zm.045 33.606c-1.99 0-3.482-1.642-3.482-3.511 0-1.915 1.492-3.511 3.482-3.511 1.854 0 3.437 1.596 3.437 3.51 0 1.87-1.583 3.512-3.437 3.512z"
                   fill="black"
                   fillRule="nonzero"
                   opacity={0.6}
                 />
               </Svg>
               </View>
                     <View style={styles.innerContainer}>
                     <Text style={{fontSize:'12'}}> Icon Sort Filter</Text>
                     <Svg xmlns="http://www.w3.org/2000/svg" width={28} height={19} >
                       <G fill="#35FFA7">
                         <G transform="translate(15 2)">
                           <Rect width={13} height={2} rx={1} />
                           <Rect width={10} height={2} y={6} rx={1} />
                           <Rect width={7} height={2} y={12} rx={1} />
                         </G>
                         <Path d="M6.015 0a1 1 0 0 1 1 1v13.668l3.282-3.203a1 1 0 0 1 1.304-.08l.093.08.028.028a.968.968 0 0 1-.016 1.37l-4.982 4.863a.996.996 0 0 1-.64.283h-.167a.997.997 0 0 1-.64-.283L.293 12.862l-.016-.016a.968.968 0 0 1 .016-1.37l.012-.011a.998.998 0 0 1 1.397 0l3.303 3.224V1a1 1 0 0 1 1-1h.01z" />
                       </G>
                     </Svg>
                     </View>
               </View>
                     <View style={styles.container}>

                <View style={styles.innerContainer}>
                     <Text style={{fontSize:'12'}}> Icon Menu Home</Text>
                     <Svg xmlns="http://www.w3.org/2000/svg" width={18} height={18}>
                       <Path
                         fill="none"
                         stroke="blue"
                         d="M8.998.5c.11 0 .218.046.303.13l8.187 8.147a.032.032 0 0 1 .01.018.083.083 0 0 1-.003.047l-.004.012h-2.145v8.065c0 .166-.06.318-.158.428a.44.44 0 0 1-.32.153h-2.935a.432.432 0 0 1-.32-.153.634.634 0 0 1-.159-.428v-5.904H6.541v5.904a.634.634 0 0 1-.158.428.437.437 0 0 1-.32.153H3.128a.434.434 0 0 1-.32-.153.634.634 0 0 1-.158-.428V8.854l-2.1-.001L8.694.631a.439.439 0 0 1 .222-.122z"
                       />
                     </Svg>
                     </View>
                     <View style={styles.innerContainer}>
                     <Text style={{fontSize:'12'}}> Icon Guide Header</Text>
                     <Svg xmlns="http://www.w3.org/2000/svg" width={27} height={18} >
                       <G fill="blue" fillRule="evenodd">
                         <Rect width={6.923} height={2.769} x={19.78} rx={1} />
                         <Rect width={7.121} height={2.769} x={19.78} y={7.615} rx={1} />
                         <Rect width={7.121} height={2.769} x={19.78} y={15.231} rx={1} />
                         <Rect width={6.923} height={2.769} x={9.89} rx={1} />
                         <Rect width={7.121} height={2.769} x={9.89} y={7.615} rx={1} />
                         <Rect width={7.121} height={2.769} x={9.89} y={15.231} rx={1} />
                         <Rect width={6.923} height={2.769} rx={1} />
                         <Rect width={7.121} height={2.769} y={7.615} rx={1} />
                         <Rect width={7.121} height={2.769} y={15.231} rx={1} />
                       </G>
                     </Svg>
                     </View>
                     <View style={styles.innerContainer}>
                        <Text style={{fontSize:'12'}}> Icon Live</Text>
                          <Svg xmlns="http://www.w3.org/2000/svg" width={19} height={18}>
                            <Path
                              fill="blue"
                              d="m8.154 13.58 3.326-2.085c.56-.347.56-1.234 0-1.59L8.154 7.82c-.56-.347-1.254.1-1.254.796v4.16c0 .703.694 1.151 1.254.804zm9.119-10.125h-6.555L13.257.782a.467.467 0 0 0 0-.646.414.414 0 0 0-.613 0L9.5 3.446 6.356.135a.414.414 0 0 0-.613 0 .467.467 0 0 0 0 .646l2.54 2.673H1.726C.777 3.455 0 4.264 0 5.273v10.909C0 17.182.777 18 1.727 18h15.546c.95 0 1.727-.818 1.727-1.818V5.272c0-1.008-.777-1.817-1.727-1.817zM17.1 17H1.9c-.522 0-.95-.488-.95-1.083V5.5c0-.596.428-1.083.95-1.083h15.2c.523 0 .95.487.95 1.083v10.417c0 .595-.427 1.083-.95 1.083z"
                            />
                          </Svg>
                        </View>
                     <View style={styles.innerContainer}>
                        <Text style={{fontSize:'12'}}> Icon Detail</Text>
                     <Svg xmlns="http://www.w3.org/2000/svg" width={28} height={24} >
                       <G fill="black" fillRule="evenodd">
                         <G transform="translate(0 4)">
                           <Path d="M20 0c2.21 0 4 1.79 4 4v12c0 2.21-1.79 4-4 4H4c-2.21 0-4-1.79-4-4V4c0-2.21 1.79-4 4-4h16zm0 2H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V4a2 2 0 0 0-2-2z" />
                           <Rect width={12} height={2} x={6} y={9} rx={1} />
                           <Rect width={6} height={2} x={6} y={5} rx={1} />
                           <Rect width={12} height={2} x={6} y={13} rx={1} />
                         </G>
                         <Path d="M24 0c2.21 0 4 1.79 4 4v12c0 2.21-1.79 4-4 4h-1v-2h1a2.001 2.001 0 0 0 1.995-1.85L26 16V4a2 2 0 0 0-2-2H8a2.001 2.001 0 0 0-1.995 1.85L6 4v1H4V4c0-2.21 1.79-4 4-4h16z" />
                       </G>
                     </Svg>
                    </View>
                 </View>
            </View>
          </View>
    );
  }
}


const styles = StyleSheet.create({
    top: {
      marginTop: 30,
      flexDirection: 'column',
      alignContent:'center',
      justifyContent:'center',
      backgroundColor: "lightyellow",
      borderColor:'green',
      borderWidth:2,
      left:150,
      width:600,
     paddingBottom:20,
    },
  container: {
   flexDirection: 'row',
    marginTop: 30,
    paddingBottom:10,
    alignContent:'space-around',
    backgroundColor: "lightyellow",
    borderColor:'green',
    borderWidth:2,
    left:50,
    width:500
  },
  innerContainer: {
    marginTop: 10,
    marginRight: 20,
    left:10,
    paddingBottom:10,
  }
});
AppRegistry.registerComponent('SimpleViewApp', () => CircleIcons );
