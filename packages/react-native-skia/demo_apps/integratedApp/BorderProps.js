/**
 * This Component Shows the Border properties of View.
*/

import React, {useEffect} from 'react'
import {View, AppRegistry, Text, StyleSheet, Animated} from 'react-native'
import Config from './config.json'

let resolution = Config.resolution;
let textfontSize = Config.resolution.headerFont.fontSize;

const BorderProps = (props) => {

	let text = <Text /> 
	let subViewWidth= resolution.maincontainer.width * 0.6;
	let subViewHeight= resolution.maincontainer.height * 0.6;
	let bgColor= '';
	var mVbgColor = Config.main.focusBackground;

	let borderRadius=0;
	let borderWidth=0;

	let borderLeftColor = '';
	let borderRightColor = '';
	let borderBottomColor = '';
	let borderTopColor = '';

	let borderTopWidth = 0;
	let borderRightWidth = 0;
	let borderLeftWidth = 0;
	let borderBottomWidth = 0;
	let justifyContent = 'center';
	let alignItems = 'center';

	let value = new Animated.ValueXY({x:0,y:0})

	useEffect( () =>   {
		console.log("---moveRect:" + JSON.stringify(value))
		if(props.flag != 0 && props.flag != 1) {
			Animated.timing(value,{
				toValue: {x:subViewWidth * .3, y:subViewHeight * .25},
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


    if(props.flag === 0) {
		if(props.bg === 1) {
			mVbgColor = Config.main.subtileFocus;
			mVbgColor = Config.main.subtileFocus;
			bgColor = Config.main.subtileFocus;
		} else {
			mVbgColor = Config.main.tilesBackground;
			mVbgColor = Config.main.tilesBackground;
			bgColor= Config.main.tilesBackground ;
		}
		text = 
			<Text 
				style= {[
					styles.textStyle, 
					{
						color:'white', 
						fontSize:textfontSize,
						textShadowRadius:0,
						textShadowColor:Config.main.textBackground, 
						textShadowOffset:{width:3,height:3}
					}
				]}
			> 
				Border Properties 
			</Text>
    	console.log("j = ", justifyContent, alignItems);
    }else if(props.flag === 1) {
		text = <Text> With Border </Text>;
		bgColor='#1a7599' ;
		borderWidth = 30 ;
		borderLeftColor ='#2196c4';
		borderRightColor ='#2196c4';
		borderBottomColor ='#51b8e1';
		borderTopColor ='#51b8e1'; 
		borderTopWidth= 30; 
		borderRightWidth=30;
		borderLeftWidth=30;
		borderBottomWidth= 30;
		justifyContent = 'flex-start';
		alignItems = 'flex-start';
    }  else if(props.flag === 2) {
		text = <Text> With different border width </Text>;
		bgColor='#1a7599' ;
		borderLeftColor ='#2196c4';
		borderRightColor ='#2196c4';
		borderBottomColor ='#51b8e1';
		borderTopColor ='#51b8e1';
		borderTopWidth= 15; 
		borderRightWidth=30;
		borderLeftWidth=40;
		borderBottomWidth= 50;
		justifyContent = 'flex-start';
		alignItems = 'flex-start';
    } else if(props.flag === 3){
		text = <Text> With Border radius </Text>
		bgColor ='#1a7599';
		borderLeftColor='#2196c4';
		borderRightColor='#2196c4';
		borderBottomColor='#51b8e1';
		borderTopColor='#51b8e1';  
		borderTopWidth= 30; 
		borderRightWidth=30;
		borderLeftWidth=30;
		borderBottomWidth= 30;
		borderRadius = 20;
		justifyContent = 'center';
		alignItems ='center';
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
						borderWidth:borderWidth,
						borderLeftColor:borderLeftColor,
						borderRightColor:borderRightColor,
						borderBottomColor:borderBottomColor,
						borderTopColor:borderTopColor, 
						borderTopWidth: borderTopWidth, 
						borderRightWidth:borderRightWidth,
						borderLeftWidth:borderLeftWidth,
						borderBottomWidth: borderBottomWidth,
						borderRadius: borderRadius,
						justifyContent: 'center',
						alignItems: 'center'
					}}
				>
					{text} 
				</View> 
			</Animated.View>
    	</View>
    );
        
}

const styles = StyleSheet.create({ 
	MainContainer: {
		width: resolution.maincontainer.width,
		height: resolution.maincontainer.height,
	}, 
	textStyle: {
		fontSize: textfontSize - 5,
		color: 'black', 
		fontWeight: 'bold',
		margin: 10
	}
})

AppRegistry.registerComponent('BorderProps', () => BorderProps);
export default BorderProps
