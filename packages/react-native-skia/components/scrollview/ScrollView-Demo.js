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

const NUM_ITEMS = 20;

const windowSize = Dimensions.get('window');

const SimpleViewApp = React.Node = () => {
   let svRef = React.useRef();
   let setScrollOffsetRef = React.useRef();
   let setNumItemsRef = React.useRef();
   let offset=0;

   let [isHorizontal,toggleScrollType] = React.useState(true);
   let [isFocusableItems,toggleFocusableItems] = React.useState(true);
   let [numItems,setNumOfItems] = React.useState(20);
   let [scrollEventData,setScrollEventData] = React.useState("");
   let [scrollContentSizeEventData,setScrollContentSizeEventData] = React.useState("");

   const makeItems: (nItems: number, styles: any) => Array<any> = (
       nItems: number,
       styles,
   ): Array<any> => {
       const items = [];
       for (let i = 0; i < nItems; i++) {
          if(isFocusableItems) {
               items[i] = (
                  <TouchableOpacity key={i} style={styles}>
                     <Text>{i}</Text>
                  </TouchableOpacity>
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

   const items = makeItems(numItems, styles.itemWrapper);

   const scrollToEnd = () => {svRef.current.scrollToEnd({animated:false})}

   const scrollTo = () => {
      if(isHorizontal) {
         svRef.current.scrollTo({x:offset,y:0,animated:false})
      } else {
         svRef.current.scrollTo({x:0,y:offset,animated:false})
      }
   }

   const setOffset = (e) => {
      offset = parseInt(e.nativeEvent.text);
      scrollTo();
   }

   const setNumItems = (e) => {
      setNumOfItems(parseInt(e.nativeEvent.text))
   }

   const onScrollEvent = (e) => {
      console.log("ScrollEvent : " , e.nativeEvent);
      setScrollEventData(JSON.stringify(e.nativeEvent));
   }

   const onScrollContentSizeChangeEvent = (width,height) => {
      console.log("ScrollContentSizeChange : " + width + "-" + height);
      setScrollContentSizeEventData("width:" + width + ",height:" + height);
   }

   const setScrollType = () => {
       offset=0;
       scrollTo();
       toggleScrollType(!isHorizontal);
   }

   const mainView = () => {
      return(
       <View style={{flex:1,flexDirection:'row',justifyContent:'center',alignItems:'center',width:windowSize.width,height:windowSize.height,backgroundColor:'darkslategray'}}>
         <View style={styles.mainView}>

           <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={setScrollType} >
             <Text style={{margin:5,fontSize:20,color:'darkgoldenrod'}}>{'HorizontalScroll:' + isHorizontal}</Text>
           </TouchableHighlight>

           <View style={{flexDirection:'row'}}>
             <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={()=>{setNumItemsRef.current.focus()}} >
               <Text style={{margin:5,fontSize:20,color:'darkgoldenrod'}}>{'No of Items'}</Text>
             </TouchableHighlight>
             <TextInput ref={setNumItemsRef} defaultValue={'20'} onSubmitEditing={setNumItems} style={[styles.controlButton,{color:'darkgrey'}]}/>
           </View>

           <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => {toggleFocusableItems(!isFocusableItems)}} >
             <Text style={{margin:5,fontSize:20,color:'darkgoldenrod'}}>{'FocusableItems:' + isFocusableItems}</Text>
           </TouchableHighlight>

           <View style={{flexDirection:'row'}}>
              <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={() => {setScrollOffsetRef.current.focus()}} >
                 <Text style={{margin:5,fontSize:20,color:'darkgoldenrod'}}>{'ScrollTo'}</Text>
              </TouchableHighlight>
              <TextInput ref={setScrollOffsetRef} defaultValue={'0'} onSubmitEditing={setOffset} style={[styles.controlButton,{color:'darkgrey'}]}/>
           </View>

           <TouchableHighlight underlayColor='darkseagreen' style={styles.controlButton} onPress={scrollToEnd} >
             <Text style={{margin:5,fontSize:20,color:'darkgoldenrod'}}>{'ScrollToEnd'}</Text>
           </TouchableHighlight>
        </View>

        <View style={styles.mainView}>
           <ScrollView ref={svRef} style={styles.scrollView} contentContainerStyle={{margin:5,backgroundColor:'darkolivegreen'}} horizontal={isHorizontal} onScroll={onScrollEvent} onContentSizeChange={onScrollContentSizeChangeEvent}>
             {items}
           </ScrollView>

           <View style={{borderWidth:3,borderColor:'black',height:'40%',width:'80%'}}>
              <Text style={{marginTop:5,textAlign:'center',fontWeight:'bold',fontSize:20,color:'darkgoldenrod'}}>{'EVENTS'}</Text>
              <Text style={{marginTop:5,fontSize:20,textDecorationLine:'underline',color:'darkgoldenrod'}}>{'onScroll:'}</Text>
              <Text style={{margin:5,fontSize:16,color:'darkgrey'}}>{scrollEventData}</Text>
              <Text style={{marginTop:15,fontSize:20,textDecorationLine:'underline',color:'darkgoldenrod'}}>{'onScrollContentSizeChange:'}</Text>
              <Text style={{margin:5,fontSize:16,color:'darkgrey'}}>{scrollContentSizeEventData}</Text>
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
    height:'40%',
    width:'80%',
    backgroundColor:'darksgrey',
    borderWidth:3,
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
    margin:10,
    width:250,
    height:50,
    borderWidth:3,
    borderRadius:12,
    borderColor: 'black',
  },
  mainView: {
    flexDirection:'column',
    alignItems:'center',
    justifyContent:'center',
    width:'50%',
    height:'100%',
  }
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
