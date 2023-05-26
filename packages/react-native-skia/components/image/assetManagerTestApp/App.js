import React from "react";

import {
  View,
  StyleSheet,
  TouchableOpacity,
  Text,
  Image,
  ImageBackground,
  AppRegistry,
} from "react-native";

import FastImage from "react-native-fast-image";

const SimpleViewApp = () => {
const [Imagewidth, setWidth] = React.useState(0);
const [Imageheight, setHeight] = React.useState(0);
const [Imageuri, setUri] = React.useState("");
const [ImageError,setError] = React.useState("");

return(
<View style={styles.MainContainer}>
  <View style={styles.innerContainer}>
    <View style={styles.itemContainer}>
      <Text>Please navigate here to  getSize testing </Text>
      <TouchableOpacity style={styles.button}
        onFocus={(ne) => {
            Image.getSize("1", (width, height) => {
              console.log(
                "image name called 1" + "width: " + width + " height: " + height
              );
              setWidth(width);
              setHeight(height);
              setUri("1");
            });
            setUri("1");
          }
        }
        onBlur={()=>{ setWidth(0);
              setHeight(0);
              setUri("");
            }
        }
      >
        <Text style={styles.text}> 
          getsize with out path and extention
        </Text>
      </TouchableOpacity>
      <TouchableOpacity
        style={styles.button}
        onFocus={(ne) => {
            Image.getSize(
              "https://cdn.pixabay.com/photo/2015/04/23/22/00/tree-736885_640.jpg",
              (width, height) => {
                console.log("image name called 1" + "width: " +width + " height: " +height
              );
              setWidth(width);
              setHeight(height);
              setUri(
              "https://cdn.pixabay.com/photo/2015/04/23/22/00/tree-736885_640.jpg"
              );
            });
            setUri(
              "https://cdn.pixabay.com/photo/2015/04/23/22/00/tree-736885_640.jpg"
              );
          }
        }
        onBlur={
          ()=>{
           setWidth(0);
           setHeight(0);
           setUri("");
          }
        }
      >
          <Text style={styles.text} >getSize Remote</Text>
      </TouchableOpacity>
      <TouchableOpacity
        style={styles.button}
        onFocus={(ne) => {
            Image.getSize(
              "file:///assets/components/image/1.png",
              (width, height) => {
                console.log( "image name called 1" + "width: " + width +" height: " +height);
                setWidth(width);
                setHeight(height);
                setUri("file:///assets/components/image/1.png");
              }
            );
            setUri("file:///assets/components/image/1.png");
          }
        }
        onBlur={
          ()=>{ 
            setWidth(0);
            setHeight(0);
            setUri("");
            }
        }
      >
        <Text style={styles.text}>getSize uri - File type</Text>
      </TouchableOpacity>
      <TouchableOpacity
        style={styles.button}
        onFocus={(ne) => {
            Image.getSize(
              "1.png",
              (width, height) => {
                console.log( "image name called 1" + "width: " + width +" height: " +height);
                setWidth(width);
                setHeight(height);
                setUri("1.png");
              }
            );
            setUri("1.png");
          }
        }
        onBlur={
          ()=>{ 
            setWidth(0);
            setHeight(0);
            setUri("");
            }
        }
      >
        <Text style={styles.text} >getSize File with extention</Text>
      </TouchableOpacity>
      <TouchableOpacity
        style={styles.button}
        onFocus={(ne) => {
            Image.getSize(
              "./components/image/1.png",
              (width, height) => {
                console.log( "image name called 1" + "width: " + width +" height: " +height);
                setWidth(width);
                setHeight(height);
                setUri("./components/image/1.png");
              }
            );
            setUri("./components/image/1.png");
          }
        }
        onBlur={
          ()=>{ 
            setWidth(0);
            setHeight(0);
            setUri("");
            }
        }
      >
        <Text style={styles.text} >getSize  with relativePath</Text>
      </TouchableOpacity>
        <Text style={styles.resultText}> Result </Text>
        <Text > uri={Imageuri} </Text>
        <Text style={styles.resultText}>  width={Imagewidth} height={Imageheight} </Text>
    </View>
  </View>
  <View style={styles.innerContainer}>
     <Text> Image Uri testing </Text>
     <Image
        style={styles.tinyLogo}
        source={{ uri: "https:\/\/reactnative.dev\/img\/tiny_logo.png" }}
      />
      <Text style={styles.text}> 
        Image remote
      </Text>
      <Image
        style={styles.tinyLogo}
        source={{ uri: "1" }}
      />
      <Text style={styles.text}> 
        Image without path and extention
      </Text>
      <Image
        style={styles.tinyLogo}
        source={{ uri: "1.png" }}
      />
      <Text style={styles.text}> 
        Image name with extention
      </Text>
      <Image
        style={styles.tinyLogo}
        source={require("./components/image/1.png")}
      />
      <Text style={styles.text}> 
        Image name with extention using require
      </Text>
      <Image
        style={styles.tinyLogo}
        source={{uri:"file:://./components/image/1.png"}}
      />
      <Text style={styles.text}> 
        Image file type -- file://
      </Text>
  </View>
  <View style={styles.innerContainer}>
     <Text> Fast Image Uri testing </Text>
     <FastImage
        style={styles.tinyLogo}
        source={{ uri: "https:\/\/reactnative.dev\/img\/tiny_logo.png" }}
      />
      <Text style={styles.text}> 
        FastImage remote
      </Text>
      <FastImage
        style={styles.tinyLogo}
        source={{ uri: "1" }}
      />
      <Text style={styles.text}> 
        FastImage without path and extention
      </Text>
      <FastImage
        style={styles.tinyLogo}
        source={{ uri: "1.png" }}
      />
      <Text style={styles.text}> 
        FastImage name with extention
      </Text>
      <FastImage
        style={styles.tinyLogo}
        source={require("./components/image/1.png")}
      />
      <Text style={styles.text}> 
        FastImage name with extention using require
      </Text>
      <FastImage
        style={styles.tinyLogo}
        source={{uri:"file:://./components/image/1.png"}}
      />
      <Text style={styles.text}> 
        FastImage file type -- file://
      </Text>
  </View>
  <View style={styles.innerContainer}>
    <Text> ImageBG Uri testing </Text>
      <ImageBackground
          source={{ uri: "https:\/\/reactnative.dev\/img\/tiny_logo.png" }}
          resizeMode="cover"
          style={styles.image}
      >
        <Text style={styles.bgImageText}> 
          ImageBg with uri 
        </Text>
      </ImageBackground>

      <ImageBackground
          source={{ uri: "1" }}
          resizeMode="cover"
          style={styles.image}
      >
        <Text style={styles.bgImageText}> 
          ImageBg without path and extention 
        </Text>
      </ImageBackground>
      <ImageBackground
          source={{ uri: "1.png" }}
          resizeMode="cover"
          style={styles.image}
      >
        <Text style={styles.bgImageText}> 
          ImageBg with extention 
        </Text>
      </ImageBackground>
      <ImageBackground
          source={require("./components/image/1.png")}
          resizeMode="cover"
          style={styles.image}
      >
        <Text style={styles.bgImageText}> 
          ImageBg with require path and extention 
        </Text>
      </ImageBackground>
      <ImageBackground
          source={{uri:"file:://./components/image/1.png"}}
          resizeMode="cover"
          style={styles.image}
      >
        <Text style={styles.bgImageText}> 
          FastImage file type -- file:// 
        </Text>
      </ImageBackground>


  </View>
</View>
    );
};
const styles = StyleSheet.create({
  MainContainer: {
    flexDirection: "row",
    justifyContent: "space-between",
    padding: 20,
    margin: 20,
    height:720,
    width:1280,
    borderColor: "red",
    borderWidth: 1,
  },
  innerContainer: { 
    borderColor: "green",
    justifyContent: "space-between",
    height:500,
    width:300, 
    padding: 20, 
    margin: 20 
  },
  itemContainer: {
    borderColor: "blue",
    borderWidth: 1,
    height:500,
    width:300,
    flex: 1,
    flexDirectoin: "row",
  },
  buttonContainer: {
    borderColor: "yellow",
    borderWidth: 1,
    flex: 1,
    height:200,
    width:300,
    flexDirection: "row",
  },
  button: {
    alignItems: "center",
    backgroundColor: "white",
    padding: 10,
    width: 200,
    height:100,
    margin: 10,
    flex: 1,
    borderWidth:2,
    borderColor:"blue",
  },
  resultText:{
    color:"green",
    fontSize: 20,
    fontWeight: 'bold'
  },
  text:{
    color:"blue",
  },
  bgImageText:{
    color:"green",
    fontSize: 18,
  },
  image: { 
    justifyContent: "center",
    width: 200, 
    height:50 
  },
  tinyLogo: { 
    width: 50, 
    height: 50,
    borderWidth:2,
    borderColor:"blue", 
  },
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);    