import * as React from "react";
import { Text, View, StyleSheet,TouchableOpacity, TouchableHighlight } from "react-native";

import Carousel from "react-native-snap-carousel";
import PropChangeView from "./PropChangeView";

export default class SnapCarouselView extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      activeIndex: 3,
      carouselItems: []
    };
    for (let i = 0; i < 20; i++) {
      this.state.carouselItems.push({
        title: "Item" + (i + 1),
        text: "Text" + (i + 1)
      });
    }
  }
  _renderItem({ item, index }) {
    return (
      <TouchableHighlight underlayColor={"red"} activeOpacity={0.1}>
        <View
          style={styles.renderItemView}
        >
          <Text style={{ fontSize: 30 }}>{item.title}</Text>
          <Text>{item.text}</Text>
        </View>
      </TouchableHighlight>
    );
  }
  render() {
    let SLIDER_WIDTH = 600;
    const { carouselRef, ...carouselProps } = this.props;
    this.carouselProps = carouselProps;
    this.carouselRef = carouselRef;

    return (
      <View>
        <View style={styles.snapView} >
          <Carousel
            layout={"default"}
            ref={(ref) => {
              this.carouselRef.current = ref;
            }}
            data={this.state.carouselItems}
            sliderWidth={SLIDER_WIDTH}
            itemWidth={200}
            // auto play
            autoplay={true}
            autoplayInterval={400}
            autoplayDelay={2000}

            useScrollView={true}
            lockScrollWhileSnapping={true}
            lockScrollTimeoutDuration={1000}
            firstItem={1}
            style={{ backgroundColor: "yellow" }}
            apparitionDelay={500}
            renderItem={this._renderItem}
            {...this.carouselProps}
            onSnapToItem={(index) => {
               console.log("------onSnapToItem-------index:" + index);
              // this.setState({ activeIndex: index });
            }}
          />
        </View>

        <View 
        style={styles.methodView}>
          <View>
            <Text style={styles.textItem}>Methods</Text>
            <TouchableOpacity
              onPress={() => {
                this.carouselRef.current.snapToItem(0);
                this.carouselRef.current.startAutoplay();
              }}
              style={styles.methodButtonItem}
            >
              <Text style={styles.textItem}>Start AutoPlay</Text>
            </TouchableOpacity>

            <TouchableOpacity
              onPress={() => {
                this.carouselRef.current.stopAutoplay();
                this.carouselRef.current.snapToItem(0);
              }}
              style={styles.methodButtonItem}
            >
              <Text style={styles.textItem}>Stop AutoPlay</Text>
            </TouchableOpacity>
            <TouchableOpacity
              onPress={() => {
                this.carouselRef.current.snapToNext();
              }}
              style={styles.methodButtonItem}
            >
              <Text style={styles.textItem}>Snap Next</Text>
            </TouchableOpacity>
            <TouchableOpacity
              onPress={() => {
                this.carouselRef.current.snapToPrev();
              }}
              style={styles.methodButtonItem}>
              <Text style={styles.textItem}>Snap Prev</Text>
            </TouchableOpacity>
          </View>
          <View>
            <PropChangeView {...this.carouselProps} />
          </View>
        </View>
      </View>
    );
  }
}
const styles = StyleSheet.create({
  renderItemView:{
    backgroundColor: "floralwhite",
    borderRadius: 5,
    borderColor:'red',
    borderWidth:2,
    height: 200,
    width: 200,
    padding: 50,
    marginLeft: 5,
    marginRight: 25
  },
  snapView: {
    borderColor: "red",
    borderWidth: 2,
    backgroundColor: "rebeccapurple",
    width: 700,
    height: 400,
    padding: 10
  },
  textItem: {
    fontSize: 30
  },
  methodView:{
    display: "flex",
    flexDirection: "row"
  },
  methodButtonItem: {
    margin: 8,
    alignItems: "center",
    justifyContent: "center",
    width: 250,
    height: 50,
    paddingLeft:1,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3529"
  },
});
