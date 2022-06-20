'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  View,
  Text,
  TouchableOpacity,
  TouchableHighlight,
  TextInput,
  AppRegistry,
  Dimensions
} = require('react-native');

const windowSize = Dimensions.get('window');
let topValue = 0;
let leftValue = 0;
let bottomValue = 0;
let rightValue = 0;

const SimpleViewApp = React.Node = () => {
  let scrollViewRef = React.useRef();
  let offset=0;

  let [tiColor,setTiColor] = React.useState("darkgrey");
  let [scrollConfigs,setScrollConfigs] = React.useState(
    {
      "childItemType" : "View",
      "childItemNum" : 20,
      "_borderWidth" : 0,
      "_frameWidth"  : 800,
      "_frameHeight" : 500,
    });

  let [scrollProps,setScrollProps] = React.useState({
      "_horizontal" : true,
      "_scrollEnabled" : true,
      "_showsHorizontalScrollIndicator" : true,
      "_showsVerticalScrollIndicator" : true,
      "_persistentScrollbar" : false,
      "_indicatorStyle" : 'default',
      "_scrollIndicatorInsets" : {left:leftValue,top:topValue,right:rightValue,bottom:bottomValue},
    });

  let [scrollEventData,setScrollEventData] = React.useState(
    {
      "_onScrollEvent" : "",
      "_onContentSizeChange" : "",
      "_onLayout" : "",
    });

  const makeItems: (nItems: number, styles: any) => Array<any> = (
      nItems: number,
      styles,
    ): Array<any> => {
      const items = [];
      for (let i = 0; i < nItems; i++) {
        if(scrollConfigs["childItemType"] == "TouchableOpacity") {
          items[i] = (
            <TouchableOpacity key={i} style={styles}>
              <Text>{i}</Text>
            </TouchableOpacity>
          );
        } else if (scrollConfigs["childItemType"] == "TouchableHighlight") {
          items[i] = (
            <TouchableHighlight key={i} style={styles} onPress={()=>{}} underlayColor="darkcyan">
              <Text>{i}</Text>
            </TouchableHighlight>
          );
        } else {
          items[i] = (
            <View key={i} style={styles}>
              <Text>{i}</Text>
            </View>
          );
        }
      }
      return items;
  };

  const items = makeItems(scrollConfigs["childItemNum"], styles.itemWrapper);

  const scrollTo = (e) => {
    offset = parseInt(e.nativeEvent.text);
    if(scrollProps["_horizontal"]) {
      scrollViewRef.current.scrollTo({x:offset,y:0,animated:false})
    } else {
      scrollViewRef.current.scrollTo({x:0,y:offset,animated:false})
    }
  }

  const setScrollConfiguration = (configType,configValue) => {
    let updatedConfig={};
    if(configType == "addItem"){
      updatedConfig = {"childItemNum":scrollConfigs["childItemNum"]+1};
    } else if (configType == "removeItem") {
      if(scrollConfigs["childItemNum"] <= 1) updatedConfig = {"childItemNum":0};
      else updatedConfig = {"childItemNum":scrollConfigs["childItemNum"]-1};
    } else if (configType == "itemType") {
      if(scrollConfigs["childItemType"] == "View") updatedConfig={"childItemType":"TouchableHighlight"};
      else if(scrollConfigs["childItemType"] == "TouchableHighlight") updatedConfig={"childItemType":"TouchableOpacity"};
      else if(scrollConfigs["childItemType"] == "TouchableOpacity") updatedConfig={"childItemType":"View"};
    } else if (configType == "borderWidth") {
      if(scrollConfigs["_borderWidth"] == 10) updatedConfig={"_borderWidth" : 0};
      else updatedConfig={"_borderWidth":scrollConfigs["_borderWidth"]+1}
    }
    else if (configType == "frameWidth") updatedConfig={"_frameWidth" : configValue};
    else if (configType == "frameHeight") updatedConfig={"_frameHeight" : configValue};

    setScrollConfigs(scrollConfigs => ({...scrollConfigs,...updatedConfig}));
  }

  const setScrollProperties = (propType) => {
    let updatedProp = {}
    if(propType == "toggleHorizontal") updatedProp = {"_horizontal": !scrollProps["_horizontal"]}
    else if (propType == "toggleScrollEnabled") updatedProp = {"_scrollEnabled": !scrollProps["_scrollEnabled"]}
    else if (propType == "toggleVBar") updatedProp = {"_showsVerticalScrollIndicator": !scrollProps["_showsVerticalScrollIndicator"]}
    else if (propType == "toggleHBar") updatedProp = {"_showsHorizontalScrollIndicator": !scrollProps["_showsHorizontalScrollIndicator"]}
    else if (propType == "toggleBarColor") {
      if(scrollProps["_indicatorStyle"] == 'default') updatedProp = {"_indicatorStyle":'black'}
      else if (scrollProps["_indicatorStyle"] == 'black') updatedProp = {"_indicatorStyle" : 'white'}
      else updatedProp = {"_indicatorStyle" : 'default'}
    } else if (propType == "setIndicatorInsets") {
      updatedProp = {"_scrollIndicatorInsets":{top:topValue,left:leftValue,bottom:bottomValue,right:rightValue}}
      setTiColor("darkgrey");
    }
    setScrollProps(scrollProps => ({...scrollProps,...updatedProp}));
  }

  const setScrollEventDetails = (eventType,eventInfo) => {
    let updatedEvent = {}
    if(eventType == "onScrollEvent") updatedEvent = {"_onScrollEvent":eventInfo}
    else if (eventType == "onContentSizeChange") updatedEvent = {"_onContentSizeChange":eventInfo}

    setScrollEventData(scrollEventData => ({...scrollEventData,...updatedEvent}));
  }

  const mainView = () => {
    return(
      <View style={{flex:1,width:windowSize.width,height:windowSize.height,backgroundColor:'darkslategray'}}>
        <ScrollView ref={scrollViewRef}
              style={[styles.scrollView,{borderWidth:scrollConfigs["_borderWidth"],width:scrollConfigs["_frameWidth"],height:scrollConfigs["_frameHeight"]}]}
              scrollEnabled={scrollProps["_scrollEnabled"]}
              horizontal={scrollProps["_horizontal"]}
              showsHorizontalScrollIndicator={scrollProps["_showsHorizontalScrollIndicator"]}
              showsVerticalScrollIndicator={scrollProps["_showsVerticalScrollIndicator"]}
              persistentScrollbar={scrollProps["_persistentScrollbar"]}
              indicatorStyle={scrollProps["_indicatorStyle"]}
              scrollIndicatorInsets={scrollProps["_scrollIndicatorInsets"]}
              onScroll={(e) => setScrollEventDetails("onScrollEvent",JSON.stringify(e.nativeEvent))}
              onContentSizeChange={(width,height) => setScrollEventDetails("onContentSizeChange","width:"+width+",height:"+height)} >

            {items}
        </ScrollView>

        <View style={{flexDirection:'column'}}>
          <Text style={[styles.controlButtonText,{textDecorationLine:'underline',fontWeight:'bold',color:'darkorange',marginTop:30}]}>{'CONFIGURATIONS'}</Text>

          <View style={{flexDirection:'row',flexWrap:'wrap'}}>
            <View style={[styles.controlButton,{width:50,height:50}]}>
              <Text style={styles.controlButtonText}>{scrollConfigs["childItemNum"]}</Text>
            </View>
            <View styles={{flexDirection:'column'}}>
              <TouchableHighlight underlayColor='darkseagreen' style={[styles.controlButton,{width:150,height:30}]} onPress={() => setScrollConfiguration("addItem")}>
                <Text style={styles.controlButtonText}>{'Add Item'}</Text>
              </TouchableHighlight>
              <TouchableHighlight underlayColor='darkseagreen' style={[styles.controlButton,{width:150,height:30}]} onPress={() => setScrollConfiguration("removeItem")}>
                <Text style={styles.controlButtonText}>{'Remove Item'}</Text>
              </TouchableHighlight>
            </View>

            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollConfiguration("itemType")} >
              <Text style={styles.controlButtonText}>{'ItemType:' + scrollConfigs["childItemType"]}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollConfiguration("borderWidth")} >
              <Text style={styles.controlButtonText}>{'BorderWidth : ' + scrollConfigs["_borderWidth"]}</Text>
            </TouchableHighlight>
            <View style={styles.inputControlButton}>
              <Text style={styles.controlButtonText}>{'Frame Width'}</Text>
              <TextInput defaultValue={scrollConfigs["_frameWidth"].toString()} style={{color:'darkgrey',width:100,height:30,margin:5}} onSubmitEditing={(e) => setScrollConfiguration("frameWidth",parseInt(e.nativeEvent.text))}/>
            </View>
            <View style={styles.inputControlButton}>
              <Text style={styles.controlButtonText}>{'Frame Height'}</Text>
              <TextInput defaultValue={scrollConfigs["_frameHeight"].toString()} style={{color:'darkgrey',width:100,height:30,margin:5}} onSubmitEditing={(e) => setScrollConfiguration("frameHeight",parseInt(e.nativeEvent.text))}/>
            </View>
          </View>

          <Text style={[styles.controlButtonText,{textDecorationLine:'underline',fontWeight:'bold',color:'darkorange',marginTop:10}]}>{'PROPERTIES'}</Text>
          <View style={{flexDirection:'row',flexWrap:'wrap'}}>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollProperties("toggleScrollEnabled")} >
              <Text style={styles.controlButtonText}>{'scrollEnabled : ' + scrollProps["_scrollEnabled"]}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollProperties("toggleHorizontal")} >
              <Text style={styles.controlButtonText}>{'horizontal : ' + scrollProps["_horizontal"]}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollProperties("toggleVBar")} >
              <Text style={styles.controlButtonText}>{'shows VBar : ' + scrollProps["_showsVerticalScrollIndicator"]}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollProperties("toggleHBar")} >
              <Text style={styles.controlButtonText}>{'shows HBar : ' + scrollProps["_showsHorizontalScrollIndicator"]}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => setScrollProperties("toggleBarColor")} >
              <Text style={styles.controlButtonText}>{'indicatorstyle : ' + scrollProps["_indicatorStyle"]}</Text>
            </TouchableHighlight>
            <View style={styles.inputControlButton}>
              <TouchableHighlight underlayColor='darkseagreen' style={[styles.controlButton,{borderWidth:0,height:30}]} onPress={() => setScrollProperties("setIndicatorInsets")} >
                <Text style={styles.controlButtonText}>{'IndicatorInsets [LTRB]'}</Text>
              </TouchableHighlight>
              <TextInput defaultValue={leftValue.toString()} onEndEditing={(e) => {leftValue=parseInt(e.nativeEvent.text);setTiColor("white")}} style={{color:tiColor,width:75,height:30,margin:5}}/>
              <TextInput defaultValue={topValue.toString()} onEndEditing={(e) => {topValue=parseInt(e.nativeEvent.text);setTiColor("white")}} style={{color:tiColor,width:75,height:30,margin:5}}/>
              <TextInput defaultValue={rightValue.toString()} onEndEditing={(e) => {rightValue=parseInt(e.nativeEvent.text);setTiColor("white")}} style={{color:tiColor,width:75,height:30,margin:5}}/>
              <TextInput defaultValue={bottomValue.toString()} onEndEditing={(e) => {bottomValue=parseInt(e.nativeEvent.text);setTiColor("white")}} style={{color:tiColor,width:75,height:30,margin:5}}/>
            </View>
          </View>

          <Text style={[styles.controlButtonText,{textDecorationLine:'underline',fontWeight:'bold',color:'darkorange',marginTop:10}]}>{'METHODS'}</Text>
          <View style={{flexDirection:'row',flexWrap:'wrap'}}>
            <View style={styles.inputControlButton}>
              <Text style={styles.controlButtonText}>{'ScrollTo'}</Text>
              <TextInput defaultValue={'0'} onSubmitEditing={scrollTo} style={{color:'darkgrey',width:100,height:30,margin:5}}/>
            </View>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() =>{scrollViewRef.current.scrollToEnd({animated:false})}} >
              <Text style={styles.controlButtonText}>{'ScrollToEnd'}</Text>
            </TouchableHighlight>
            <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() =>{scrollViewRef.current.flashScrollIndicators()}} >
              <Text style={styles.controlButtonText}>{'flashScrollIndicators'}</Text>
            </TouchableHighlight>
          </View>

          <Text style={[styles.controlButtonText,{textDecorationLine:'underline',fontWeight:'bold',color:'darkorange',marginTop:10}]}>{'EVENTS'}</Text>
          <View style={{flexDirection:'column',flexWrap:'wrap'}}>
            <Text style={{margin:5,fontSize:16,textDecorationLine:'underline',color:'darkgoldenrod'}}>{'onScroll:'}</Text>
            <Text style={{margin:5,fontSize:16,color:'darkgrey'}}>{scrollEventData["_onScrollEvent"]}</Text>
            <Text style={{margin:5,fontSize:16,textDecorationLine:'underline',color:'darkgoldenrod'}}>{'onScrollContentSizeChange:'}</Text>
            <Text style={{margin:5,fontSize:16,color:'darkgrey'}}>{scrollEventData["_onContentSizeChange"]}</Text>
          </View>
        </View>

      </View>
    )
  }

  return (
    mainView()
  );
}

const styles = StyleSheet.create({
  scrollView: {
    margin: 10,
    backgroundColor:'darkgrey',
    borderColor:'black'
  },
  itemWrapper: {
    backgroundColor: 'darkseagreen',
    alignItems: 'center',
    justifyContent:'center',
    borderRadius: 5,
    borderWidth: 2,
    borderColor: 'black',
    margin:10,
    width:300,
    height:300,
  },
  controlButton: {
    margin:5,
    width:250,
    height:50,
    borderWidth:2,
    borderRadius:12,
    borderColor: 'black',
  },
  controlButtonText: {
    fontSize:16,
    color:'darkgoldenrod',
    //textAlign:'center',
    padding:10,
  },
  inputControlButton: {
    margin:5,
    height:50,
    borderWidth:2,
    borderRadius:12,
    borderColor: 'black',
    flexDirection:'row'
  },
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
