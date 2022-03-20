/**
 * This Component Shows the Image properties.
*/

import React from 'react'
import {View, Image} from 'react-native'
import Config from './config.json'

let resolution = Config.resolution;

const ImageProps = (props) => {

    let index = 1;
    let bwidth = 0;
    let opacityValue = 1;
    let bColor = '';
    let bgColor = '';
    let width = resolution.maincontainer.width * 0.7;
    let height = resolution.maincontainer.width * 0.6;
    let resizeCase = 2;
  
    const imagePath = [
		require('./images/logo256.png'),
		require('./images/logo128.png'),
		require('./images/tImage13.png') 
    ]
  
    let resizemodes = [
		"center",
		"cover",
		"contain",
		"stretch",
		"repeat",
    ];

	if(props.flag === 0){
		index = 2;
		width = resolution.maincontainer.width * 0.76;
		if(props.bg === 1) {
			bgColor = Config.main.subtileFocus;
			bColor = Config.main.subtileFocus;
		} else {
			bgColor= Config.main.tilesBackground;
			bColor = Config.main.tilesBackground;
		}
	} else if(props.flag === 1){
		index = 1;
		bgColor = Config.main.focusBackground;
		bwidth = 0;
		resizeCase = 0;
	} else if( props.flag === 2){
		index = 1;
		resizeCase = 2;
		opacityValue = 0.5;
		bgColor = Config.main.focusBackground;
	} else if(props.flag === 3) {
		index = 1;
		resizeCase = 4;
		bwidth = 10;
		bColor = Config.animation.backgroundColor;
    	bgColor = Config.main.focusBackground;
	} 

  return(
	<View 
		style={{
			justifyContent:'center', alignItems:'center', 
			width:resolution.maincontainer.width, 
			height:resolution.maincontainer.height,
     		backgroundColor:bgColor, borderWidth:5, 
			borderRadius:10, borderColor:bgColor
		}}
	> 
		<Image 
			style={{width:width,height:height,
			borderColor:bColor,  borderWidth:bwidth, 
			opacity:opacityValue, backgroundColor:bgColor,alt:'Hello',
			resizeMode:resizemodes[resizeCase]}}
			source={imagePath[index]}
		>
		</Image>
	</View>
  );
}

export default ImageProps
