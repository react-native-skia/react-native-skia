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

return(
<View style={styles.MainContainer}>
  <ImageBackground
        source={{ uri: "1" }}
        resizeMode="cover"
        style={styles.image}
      >
  <View style={styles.innerContainer}>
    <View style={styles.itemContainer}>
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
          <Text>getSize Remote</Text>
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
        <Text>getSize uriFile type</Text>
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
        <Text>getSize File with extention</Text>
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
        <Text>getSize  with relativePath</Text>
      </TouchableOpacity>
        <Text style={styles.resultText}> Result </Text>
        <Text > uri={Imageuri} </Text>
        <Text style={styles.resultText}> width={Imagewidth} height={Imageheight} </Text>
    </View>
  </View>
  </ImageBackground>
  <View style={styles.innerContainer}>
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
</View>
    );
};
const styles = StyleSheet.create({
  MainContainer: {
    flexDirection: "row",
    justifyContent: "space-between",
    padding: 20,
    margin: 20,
    height:800,
    width:1500,
    borderColor: "red",
    borderWidth: 1,
  },
  innerContainer: { 
    borderColor: "green",
    justifyContent: "space-between",
    height:500,
    width:400, 
    padding: 20, 
    margin: 20 
  },
  itemContainer: {
    borderColor: "blue",
    borderWidth: 1,
    flex: 1,
    flexDirectoin: "row",
  },
  buttonContainer: {
    borderColor: "yellow",
    borderWidth: 1,
    flex: 1,
    height:200,
    width:400,
    flexDirection: "row",
  },
  
  button: {
    alignItems: "center",
    backgroundColor: "white",
    padding: 10,
    width: 300,
    height:100,
    margin: 10,
    flex: 1,
  },
  resultText:{
    color:"green",
    fontSize: 20,
    fontWeight: 'bold'
  },
  text:{
    color:"black",
  },
  image: { 
    justifyContent: "center" 
  },
  tinyLogo: { width: 50, height: 50 },
});

export default SimpleViewApp;
AppRegistry.registerComponent('SimpleViewApp', () => SimpleViewApp);    