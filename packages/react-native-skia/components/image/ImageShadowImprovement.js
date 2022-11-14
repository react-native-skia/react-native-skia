import * as React from 'react';
import { View, AppRegistry, Image, Text } from 'react-native';

const SimpleViewApp = React.Node = () => {
  return (
  	<>
	  <View
	  style={{
		  flex:1,
			  flexDirection:'row',
	  
	  }}>
	  
    <View
      style={{ width: 300,
		      height: 300,
		     
		      
            }}>
	   <Image
               source={require('./cake.png')}
               style={{ width: 200, 
			                  height: 200 ,
			                        
                               shadowColor: "gray",
                               shadowOpacity: 1,
                               shadowOffset: {
                                               width: 100,
                                               height: 100}
                     }}/>
     
</View>
 <View
      style={{ width: 600,
		      height: 600,
		     
		      
            }}>
	   <Image
               source={require('./1.jpg')}
               style={{ width: 200, 
			                  height: 200 ,
			                        
                               shadowColor: "gray",
                               shadowOpacity: 1,
                               shadowOffset: {
                                               width: 50,
                                               height: 50}
                     }}/>
     
</View>
</View>
</>
  );
};
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);
