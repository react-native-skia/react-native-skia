/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed URLr the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

'use strict';


//const fs = require('fs');
const React = require('react');
const ReactNative = require('react-native');
const {View, Text} = ReactNative;
import Config from './config.json'

let fontSize1 = Config.resolution.headerFont.fontSize;

const multi_url = Config.websocket.url;
let TextSent = <Text />

let bgColor = '';

type State = {
	url1: string,
	fetchStatus1: ?string,
	socket1: ?WebSocket,
	socketState1: ?number,
	lastSocketEvent1: ?string,
	lastMessage1: ?string | ?ArrayBuffer,
	testMessage1: string,
	testExpectedResponse1: string,
	testFlag: ?number,
	...
};

class WebSocketTest extends React.Component<{...}, State> {

	state: State = {
	url1: multi_url,
	fetchStatus1: null,
	socket1: null,
	socketState1: null,
	lastSocketEvent1: null,
	lastMessage1: null,
	testMessage1: 'testMessage1',
	testExpectedResponse1: "",
	testFlag: 1,
	};

	_waitFor = (condition: any, timeout: any, callback: any) => {
		let remaining = timeout;
		const timeoutFunction = function() {
			if (condition()) {
				callback(true);
				return;
			}
			remaining--;
			if (remaining === 0) {
				callback(false);
			} else {
				setTimeout(timeoutFunction, 1000);
			}
		};
		setTimeout(timeoutFunction, 1000);
	};

	_connect1 = () => {
		console.log("URL: ", this.state.url1);
		const socket1 = new WebSocket(this.state.url1);
		console.log("Established new websocket",socket1);
		socket1.addEventListener("open", this._onSocketOpenEvent1);
		socket1.addEventListener("message", this._onSocketMessageEvent1);
		socket1.addEventListener("close", this._onSocketCloseEvent1);
        socket1.addEventListener("close", this._onSocketErrorEvent1);
		this.setState({
			socket1,
			socketState1: socket1.readyState,
			testFlag:0
		});
	};

    _onSocketErrorEvent1 = () => {
        console.log("Error in Connection");
    }
	_socketIsConnected1 = () => {
		console.log("_socketIsConnected: ", this.state.socketState1);
		return this.state.socketState1 === 1; //'OPEN'
	};

	_disconnect = () => {
		console.log("_disconnect",this.state.socket1);
		if (!this.state.socket1) {
			return;
		}
		this.state.socket1.close();
	};


	_onSocketOpenEvent1 = (event: any) => {
		console.log("onsocket open event", event);
		const state: any = {
			socketState1: 1, //event.target.readyState,
			lastSocketEvent1: "open" //event.type,
		};
		this.setState(state);
	};

	_onSocketMessageEvent1 = (event: any) => {
		console.log("onsocket message event", event.data);
		const state: any = {
			socketState1: 1, //event.target.readyState,
			lastSocketEvent1: "message",
		};
		console.log("lastmessage: ", this.state.testExpectedResponse1);
		if (event.data) {
			this._disconnect();
			setTimeout(() => { this.setState({testExpectedResponse1:event.data});}, 2000)
		}
	};

	_onSocketCloseEvent1 = (event: any) => {
		console.log("onsocket close event", event);
		const state: any = {
			socketState1: 3, //event.target.readyState,
			lastSocketEvent1: "close", //event.type,
		};
		this.setState(state);
	};

	_sendText1 = (text: string) => {
		console.log("_sendText: ", this.state.socket1);
		if (!this.state.socket1) {
			return;
		}
		let payload = "Message from Client";
		TextSent = <Text 
						style={{
							fontSize:fontSize1 - 3, 
							color:'white', 
							fontWeight:'bold'
						}}
					> 
						{payload} 
					</Text>
		this.state.socket1.send(payload);
	};

	_sendTestMessage = () => {
		
	};

	_sendTestMessage1 = () => {
		this._sendText1(this.state.testMessage1);
	};

	_receivedTestExpectedResponse1 = () => {
		console.log("_receivedTestExpectedResponse:");
		return this.state.lastMessage1 === this.state.testExpectedResponse1;
	};

	componentDidMount() {
		console.log("props.flag",this.props.flag );
	};

	componentDidUpdate(){
		console.log("props.flag3333",this.props.flag , "test:", this.state.testFlag);
			if(this.props.flag === 2 && this.state.testFlag === 1) {
				this.testConnect1();  
			}
	};

	testSendAndReceive1: () => void = () => {	
		this._sendTestMessage1();
		this._waitFor(this._receivedTestExpectedResponse1, 5, messageReceived => {
			if (!messageReceived) {
				console.error("test Connect - message not received");
				return;
			}
		});
	};
	testConnect1: () => void = () => {
		this._connect1();
		this._waitFor(this._socketIsConnected1, 5, connectSucceeded => {
			if (!connectSucceeded) {
				console.error("test Connect - connection not succeeded");
				return;
			}
			this.testSendAndReceive1();
		});
	};

	testDisconnect: () => void = () => {
		this._disconnect();
		console.error("testDisconnect");
		this._waitFor(this._socketIsDisconnected, 5, disconnectSucceeded => {
		});
	};

	render(): React.Node {
		console.log("prop.flag11:", this.props.flag);
		if(this.props.flag === 0){
			if(this.props.bg === 1) {
				bgColor = Config.main.subtileFocus;
			} else {
				bgColor= Config.main.tilesBackground;
			}
			TextSent = <Text 
							style={{
								fontSize:fontSize1, 
								color:'white', 
								fontWeight:'bold', 
								textShadowRadius:0, 
								textShadowColor:Config.main.textBackground, 
								textShadowOffset:
								{
									width:3,
									height:3
								}
							}}
						> 
							Websocket Testing 
						</Text>
		} else if((this.props.flag === 3 || this.props.flag === 3)&& this.state.testFlag === 0) {
			bgColor = Config.main.focusBackground;
			const state: any = {
				testFlag : 1 ,
				testExpectedResponse1: ""
			};
			this.setState(state);
		} else if(this.props.flag === 1){
			bgColor = Config.main.focusBackground;
			TextSent = <Text 
							style={{
								fontSize:fontSize1 - 7, 
								color:'white', 
								fontWeight:'bold'
							}}
						>
							Click to send "Message to server"
						</Text>
		} else {
			bgColor = Config.main.focusBackground;
			TextSent = <Text 
							style={{
								fontSize:fontSize1 - 7, 
								color:'white', 
								fontWeight:'bold'
							}}
						>
							{this.state.testExpectedResponse1}
						</Text>
		}
		return  (
			<View 
				style={{
					flex:1, 
					justifyContent:'center', 
					alignItems:'center', 
					backgroundColor:bgColor, 
					borderWidth:5, 
					borderRadius:10, 
					borderColor:bgColor 
				}}
			>
				{TextSent}
			</View>
		);

	}
}

export default WebSocketTest
