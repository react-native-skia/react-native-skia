/**
 * This Component Shows the Text properties
*/

import React, {useEffect} from 'react'
import {View, Text, StyleSheet, Animated} from 'react-native'
import Config from './config.json'

let resolution = Config.resolution;
let textfontSize = Config.resolution.headerFont.fontSize;

const TextApp = (props) =>{
 
	let value = new Animated.ValueXY({x:0,y:10})  
	let text1 = <Text />;
	let text2 = <Text />;
	let text3 = <Text />;
	let text4 = <Text />;
	let text5 = <Text />;
	let text6 = <Text />;
	let text7 = <Text />;
	let textBackground = "#D00000";
	let bColor = '';
	let justifyContent = 'center';
	let alignItems = 'center';
	let margin = 0;

	useEffect( () =>   {
		console.log("---moveRect:" + textfontSize)
		if(props.flag === 3) {
			Animated.loop(Animated.timing(value,{
				toValue: {x:150, y:10},
				duration: 4000,
				velocity: 1,
				tension: 40,
				useNativeDriver:false
			}),{}).start();
		}
	})

    console.log("Flag: ", props.flag, "textfontSize:", textfontSize);
    if(props.flag === 0) {
		text1 = <Text 
					style={[
					styles.textStyle, 
					{color:'white', fontSize:textfontSize, 
					textShadowRadius:0, textShadowColor: Config.main.textBackground, 
					textShadowOffset:{width:3,height:3}}]} 
				>
					 Text Properties 
				</Text>
    if(props.bg === 1) {
    	bColor = Config.main.subtileFocus;
    } else {
		bColor = Config.main.tilesBackground;
    }
		justifyContent = 'center';
		alignItems = 'center';
		width1 = '100%';
		textBackground = '';
    } else if(props.flag === 1){
        textBackground = '';
        justifyContent = 'center';
        alignItems = 'center';
        margin = 5;
        text1 =  <Text  
					style={[
						styles.textStyle, 
						{ color:'orange' }
					]} 
				> 
					Default Letter Spacing. 
				</Text>
        text2 = <Text 
					style={[
						styles.textStyle, 
						{
							letterSpacing:5, 
							fontFamily: "Arial", 
							color:'orange' 
						}
					]}
				>
					Letter Spacing Value as 5
				</Text>
        text3 =<Text 
					style={[
						styles.textStyle, 
						{ letterSpacing:-3 , color:'orange'}
					]}
				>
					Letter Spacing Value as -3. 
				</Text>
        bColor = Config.main.focusBackground;
    } else if(props.flag === 2){
        justifyContent = 'center';
        alignItems = 'center';
        margin = 10
        bColor = Config.main.focusBackground;
        text1 = <Text style={[styles.textStyle,{ color:'white'}]}>Default lineHeight</Text>
        text2 = <Text style={[styles.textStyle, { lineHeight:40, color:'white' }]}>lineHeight:40</Text>
        text3 = <Text style={[styles.textStyle, { lineHeight:30,color:'white'}]}>lineHeight:30</Text>
        text4 = <Text style={[styles.textStyle, { lineHeight:20, color:'white'}]}>lineHeight:20</Text>
        text5 = <Text style={[styles.textStyle, { lineHeight:12, color:'white'}]}>lineHeight:12</Text>
        text6 = <Text style={[styles.textStyle, { lineHeight:5, color:'white'}]}>lineHeight:5</Text>
        text7 = <Text style={[styles.textStyle, { lineHeight:0,color:'white'}]}>lineHeight:0</Text>
    } else if(props.flag === 3){
        textBackground = '';
        justifyContent = 'center';
        alignItems = 'center';
        bColor = Config.main.focusBackground;
        let tempText = "Font Properties with font size"
        tempText = tempText.concat(" ",textfontSize - 4)	    
        text4 = <Text 
					style={{
						color:'white', 
						fontWeight:'bold', 
						fontFamily: 'Arial', 
						fontSize:textfontSize - 4, 
						margin: 20
					}}
				>
					{tempText}
				</Text>
        text5 = <Text 
					style={{
						fontWeight:'bold', 
						color:'orange', 
						fontFamily: 'Arial', 
						fontSize:textfontSize -4
					}}
				>
					fontFamily: Arial
				</Text>
        text6 = <Text 
					style={{ 
						lineHeight:40 ,  
						color:'white', 
						fontStyle:'italic', 
						fontSize:textfontSize -4
					}}
				>
					fontStyle: Italic
				</Text>
        text7 = <Text 
					style={{ 
						lineHeight:30, 
						fontWeight:'bold', 
						color:'green', 
						fontSize:textfontSize -4 
					}}
				>
					fontWeight: Bold
				</Text>
    } 


	return (
		<View 
			style={[ 
				styles.MainContainer,
				{ 
					flex:1, flexDirection:'row' ,
					justifyContent:justifyContent, 
					alignItems:alignItems, 
					backgroundColor:bColor, 
					borderWidth:5, 
					borderRadius:10, 
					borderColor:bColor,
				} 
			]}
		>
			<View 
				style={{
					justifyContent:justifyContent, 
					alignItems:alignItems, 
					margin:20
				}}
			>
				<View style={{ margin:margin, backgroundColor: textBackground}}>
					{text1}
				</View>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text2}
				</View>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text3}
				</View>
			</View>
			<View style={{justifyContent:justifyContent, alignItems:alignItems}}>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text4}
				</View>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text5}
				</View>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text6}
				</View>
				<View style={{ margin:margin, backgroundColor: textBackground }}>
					{text7}
				</View>
			</View>
		</View>
	);
}

const styles = StyleSheet.create({
	MainContainer: {
		width: resolution.maincontainer.width,
		height: resolution.maincontainer.height
	},
	textStyle: {
		fontSize: textfontSize - 4,
		color: Config.textApp.textColor, 
		fontWeight: 'bold',
		margin: 1
	}
});

export default TextApp

