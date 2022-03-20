/**
 * This Component Shows the Shadow properties of View.
*/

import React, {useEffect} from 'react'
import {View, AppRegistry, Text, StyleSheet, Animated} from 'react-native'
import Config from './config.json'

let resolution = Config.resolution;
let fontSize1 = Config.resolution.headerFont.fontSize;

const ShadowProps = (props) => {

    let text = <Text />
    var subViewWidth= resolution.maincontainer.width * .4;
    var subViewHeight=resolution.maincontainer.height * .67;
    var bgColor='';
    let left1 = 0;
    let top1 = 0;

    var mVbgColor = Config.main.focusBackground;
    var sOffsetWidth= 0;
    var sOffsetheight=0;
    var sRadius = 0;
    let sOpacity = 0;
    let sColor = Config.shadowProps.view.shadowColor;

    let borderWidth = 0;
    let borderRadius = 0;

    let justifyContent = 'center';
    let alignItems = 'center';

    let value=  new Animated.ValueXY({x:0,y:0});

    useEffect( () =>   {
        if(props.flag === 2){
			Animated.timing(value,{
				toValue: {
                            x: resolution.maincontainer.width * .29,
                            y: resolution.maincontainer.height * 0.12
                        },
				duration: 2000,
				velocity: 1,
				tension: 40,
				useNativeDriver:false
			}).start();
        } else if (props.flag === 3){
			Animated.timing(value,{
				toValue: {
                            x:resolution.maincontainer.width * 0.19, 
                            y:  resolution.maincontainer.height * -0.10
                        },
				duration: 2000,
				velocity: 1,
				tension: 40,
				useNativeDriver:false
			}).start();
        } else {
            Animated.timing(value,{
                toValue: {x:0, y:0},
                duration: 2000,
                velocity: 1,
                tension: 40,
                useNativeDriver:false
            }).start();
        }
    })

    if(props.flag == 0){
        sRadius = 0;
        sOpacity = 0;
        borderWidth = 0;
        borderRadius = 0;
        if(props.bg === 1) {
                mVbgColor = Config.main.subtileFocus;
                mVbgColor = Config.main.subtileFocus;
                bgColor = Config.main.subtileFocus;
        } else {
                mVbgColor = Config.main.tilesBackground;
                mVbgColor = Config.main.tilesBackground;
                bgColor= Config.main.tilesBackground;
        }
        top1= resolution.shadowProps.headerTextPosition.xAlign0;
        text = <Text 
            style={{
                textAlign:'center', 
                fontSize:fontSize1,
                color:'white', fontWeight:'bold', 
                textShadowRadius:0, 
                textShadowColor: Config.main.textBackground,
                textShadowOffset:{width:3,height:3}
            }}
		 >
			  Shadow Properties 
		</Text>
    } else if(props.flag === 1) {
        sRadius = 5;
        sOpacity = 1;
        borderWidth = 5;
        borderRadius = 5;
        alignItems = 'flex-start';
        justifyContent = 'flex-start';
        sOffsetWidth= subViewWidth/ 10;
        sOffsetheight= subViewHeight/ 10;
        bgColor= Config.shadowProps.view.bgColor;
        top1= resolution.shadowProps.headerTextPosition.xAlign1;
        left1 = resolution.shadowProps.headerTextPosition.yAlign1;
        text = <Text 
                    style={{
                        color:'white', 
                        fontWeight:'bold'
                    }}
                >  
                    With Shadows 
                </Text>
    } else if(props.flag === 2) {
        sRadius = 20;
        sOpacity = 1;
        borderWidth = 5;
        borderRadius = 5;
        alignItems = 'flex-start';
        justifyContent = 'flex-start';
        sOffsetWidth= subViewWidth/ 10;
        sOffsetheight= subViewHeight/ 10;
        bgColor= Config.shadowProps.view.bgColor;
        top1= resolution.shadowProps.headerTextPosition.xAlign1;
        left1 = resolution.shadowProps.headerTextPosition.yAlign1;
        text = <Text 
                    style={{
                        color:'white', 
                        fontWeight:'bold'
                    }
                }>
                    With shadow Radius 
                </Text>
    } else if(props.flag === 3){
        sOffsetWidth= 0;
        sOffsetheight=0;
        sRadius = 30;
        sOpacity = 1;
        borderWidth = 5;
        borderRadius = 5;
        sColor = "green";
        alignItems = 'center';
        justifyContent = 'center';
        sOffsetWidth= subViewWidth/ 10;
        sOffsetheight= subViewHeight/ 10;
        bgColor= Config.shadowProps.view.bgColor;
        top1=  resolution.shadowProps.headerTextPosition.xAlign3;
        text = <Text 
                    style={{
                        color:'white', 
                        fontWeight:'bold'
                    }}
                > 
                    With shadow color 
                </Text>
    }

    return (
        <View 
            style={[
                styles.MainContainer, 
                {
                    flex: 1, 
                    justifyContent:justifyContent, 
                    alignItems:alignItems, 
                    backgroundColor:mVbgColor, 
                    borderWidth:5, 
                    borderRadius:10, 
                    borderColor:mVbgColor
                }
            ]}
        >
			<Animated.View style={value.getLayout()}>
				<View
                    style={{
                        width:subViewWidth ,
                        height : subViewHeight,
                        backgroundColor: bgColor,
                        shadowOffset: {
                        width: sOffsetWidth,
                        height:sOffsetheight
                        },
                        shadowRadius:sRadius,
                        shadowColor:sColor,
                        shadowOpacity: sOpacity,
                        borderWidth:borderWidth,
                        borderRadius:borderRadius,
                        borderColor: bgColor,
                        justifyContent: 'center',
                        alignItems: 'center'
                    }}
                >
                </View>
			</Animated.View>
            <View style={{left:left1, top:top1}}>
                {text}
            </View>
        </View>
    );
    
}

const styles = StyleSheet.create({
	MainContainer: {
		width:resolution.maincontainer.width,
		height: resolution.maincontainer.height
	}
});

AppRegistry.registerComponent('ShadowProps', () => ShadowProps);
export default ShadowProps

