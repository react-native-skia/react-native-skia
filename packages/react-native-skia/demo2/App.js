import React, { useState, useRef, useEffect} from "react";
import { Pressable, AppRegistry, Text, View, StyleSheet, ImageBackground, Image, Animated} from "react-native";

import Config from './config.json'

let tilesBackground = Config.main.tilesBackground;
let shadowColor = Config.main.shadowColor;
let resolution = Config.resolution;

const imagePath = [
	require('./images/poster1.jpg'),
	require('./images/poster2.jpg'),
	require('./images/poster3.jpg'),
	require('./images/poster4.jpg'),
	require('./images/poster5.jpg'),
	require('./images/poster6.jpg'),
	require('./images/poster7.jpg'),
	require('./images/poster8.jpg'),
]

	const title = [
		"Frozen II",
		"Bodied",
		"Black Panther",
		"The Lion King",
		"Joker",
		"The Addams Family",
		"Accelaration",
		"Rambo: Last Blood",
	]
	const genre = [
		"Musical Comedy",
		"Comedy",
		"Action",
		"Adventure",
		"Crime drama",
		"Comedy",
		"Action",
		"Action",
	]

	const duration = [
		"1h  43m",
		"2h  01m",
		"2h  02m",
		"1h  58m",
		"2h  02m",
		"1h  27m",
		"1h  25m",
		"1h  29m",
	]

	const year = [
		"2019",
		"2017",
		"2018",
		"2019",
		"2019",
		"2019",
		"2019",
		"2019",
	]
	const description = [
	"Elsa the Snow Queen has an extraordinary gift -- the power to create ice and snow. But no matter how happy she is to be surrounded by the people of Arendelle, Elsa finds herself strangely unsettled.",

	"A progressive graduate student finds success and sparks outrage when his interest in battle rap as a thesis subject turns into a competitive obsession.",

	"After the death of his father, T'Challa returns home to the African nation of Wakanda to take his rightful place as king. When a powerful enemy suddenly reappears....",

	"Betrayed and exiled from his kingdom, lion cub Simba must figure out how to grow up and take back his royal destiny on the plains of the African savanna.",

	"Isolated, bullied and disregarded by society, failed comedian Arthur Fleck begins a slow descent into madness as he transforms into the criminal mastermind known as the Joker.",

	"Members of the mysterious and spooky Addams family -- Gomez, Morticia, Pugsley, Wednesday, Uncle Fester and Grandma -- encounter a shady TV personality who despises their eerie hilltop mansion.",

	"When an evil crime lord is double-crossed by his most trusted operative, he kidnaps her young son.",

	"Vietnam War veteran John Rambo tries to find some semblance of peace by raising horses on a ranch in Arizona. He's also developed a special familial bond with a woman named Maria and her teenage   granddaughter Gabriela.",

	"A new king rises in the north; a Khaleesi finds new hope in the season finale."
	]
  
const MyComponent = (props) =>  {

	let myRef = useRef(null);
	let width = resolution.maincontainer.width;
	let height = resolution.maincontainer.height;
	let pos = 'absolute';
	let [state, setState] = useState({
		bw:0,
		color1: 'red',
		bg: tilesBackground,
		sowidth: 0,
		soheight: 0,
    bwidth:0
	});

	const onBlur = (e) => {
		console.log("onBlur---------" )
		setState({
			bw:0,
			bg:tilesBackground,
			sowidth: 0,
			soheight: 0,
			bwidth:0
		});
	}

	const onFocus = (e) => {
		console.log("**** focus props.layout: ********");
		props.change((props.count));
		setState({
		sowidth: 0,
			soheight: 15,
			bwidth:5
		});
	}


	return (
		<Pressable isTVSelectable='true' ref={myRef} 
			onBlur={onBlur} 
			onFocus={onFocus} 
			style={{zIndex: 100, 
				borderWidth: state.bw, 
				marginTop:50, position: pos, 
				left: props.layout.x, top:props.layout.y, 
				backgroundColor:state.bg, 
				height:props.layout.h? props.layout.h:height, 
				width:props.layout.w? props.layout.w:width, 
				borderRadius:10, 
				shadowOffset: {
					width: state.sowidth,
					height: state.soheight
				},
				shadowRadius:10,
				shadowColor:shadowColor,
				shadowOpacity: 1
			}}
		>
			<Image 
				style={{width:width,height:height,
				borderColor:'white',  borderWidth:state.bwidth, 
				resizeMode:'stretch'}}
				source={imagePath[props.count]}
			> 
      		</Image>
		</Pressable>
	);
}

const SimpleViewApp = () => {

    let configList = resolution.tilePosition
	let list1 = [
					{x:configList.x1,y:configList.y1},{x:configList.x2,y:configList.y1},{x:configList.x3, y:configList.y1},{x:configList.x4, y:configList.y1},
					{x:configList.x1,y:configList.y2},{x:configList.x2,y:configList.y2},{x:configList.x3, y:configList.y2},{x:configList.x4, y:configList.y2}
				];
        
	let [index, setIndex] = useState(0);
	let fadeAnim = new Animated.Value(0.1);
	function changeBackground (value) {
		console.log("ChangeBackground:",value);
		setIndex(value);
	}

	useEffect(() => {
		Animated.timing(fadeAnim, {
			toValue: 1,
			duration: 3000,
			useNativeDriver: Config.useNativeDriver
		}).start();
	})

	const addItems = () => {
		var arr = [];
		for (var i = 0; i<list1.length; i++){
			console.log("======================random left:" +  list1[i].x + "  top:" + list1[i].y );
			arr.push(<MyComponent layout={list1[i]} count={i} change={changeBackground}></MyComponent>);
		}
		return arr;
	}

	let text1 = duration[index].concat("  |  ", genre[index], "  |  ", year[index])
	return (
		<ImageBackground style = {styles.backgroundimage} source={require('./images/bg.jpg')} resizeMode='cover'>
			{addItems()}
			<Animated.View  
				style={{ 
					zIndex:-1, width:resolution.posterView.width, 
					height:resolution.posterView.height, left:resolution.posterView.left, 
					top:resolution.posterView.top,
					shadowOffset: {
						width: 5,
						height: 30
					},
					shadowRadius:10,
					shadowColor:shadowColor,
					shadowOpacity: 1,
					opacity:fadeAnim
				}}
			>								
				<ImageBackground source={imagePath[index]} resizeMode='stretch'  style={{ zIndex:1, width:resolution.posterView.width, height:resolution.posterView.height}}>
				</ImageBackground>  
			</Animated.View>
      <View style={{ position:'absolute',width:resolution.descriptionView.width, height:resolution.descriptionView.height, top:resolution.descriptionView.top, left:resolution.descriptionView.left}}>
        <Text style={{ color:'white', fontSize:23, marginBottom: 5, fontWeight:'bold',textShadowRadius:1, textShadowColor:'black'}}>{title[index]}</Text>
        <Text style={{ color:'white', fontSize:13, marginBottom: 10, fontWeight:'normal'}}>{text1}</Text>
        <Text style={{color:'white', fontSize:20, fontWeight:'normal', lineHeight:25}}>{description[index]}</Text>
      </View>
			<View  style={{ position:'absolute' , width:resolution.subHeaders.width, height:resolution.subHeaders.height, margin:10, top:resolution.subHeaders.top1, left:resolution.subHeaders.left, justifyContent:'flex-end'}}>
				<Text style={{color:'white', fontSize:18, fontWeight:'bold', textShadowRadius:5, textShadowColor:'black'}}>Continue Watching</Text>
			</View>
			<View  style={{ position:'absolute', width:resolution.subHeaders.width, height:resolution.subHeaders.height, margin:10, top:resolution.subHeaders.top2, left:resolution.subHeaders.left, justifyContent:'flex-end'}}>
				<Text style={{color:'white', fontSize:18, fontWeight:'bold'}}>Trending</Text>
			</View>
		</ImageBackground>
	);
}

const styles = StyleSheet.create({
	backgroundimage: {
		flex: 1,
		justifyContent: "flex-start",
		alignItems:'flex-start',
		width: resolution.appSize.width,
		height:resolution.appSize.height
	}
});

AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);

export default SimpleViewApp

