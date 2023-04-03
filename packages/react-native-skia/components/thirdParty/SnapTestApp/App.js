import * as React from "react";
import {
  AppRegistry,
  StyleSheet,
  Text,
  View,
  TouchableHighlight
} from "react-native";

import SnapCarouselView from "./snapCarouselView";

function getRandomInt(max) {
  return Math.floor(Math.random() * max);
}

const defaultCarouselProps = {
  itemWidth: 200,
  sliderWidth: 600,
  layout: "default",
  activeSlideAlignment: "center",
  loop: false,
  loopClonesPerSide: 3,
  layoutCardOffset: 18,
  vertical: false,
  firstItem: 0,
  inactiveSlideOpacity: 0.7,
  inactiveSlideScale: 0.9,
  inactiveSlideShift: 0,
  containerCustomStyle: { marginTop: 2 }
};

const carouselProps = [
  {
    name: "StopAutoplay",
    getProperties: () => {
      let prop = {};
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.stopAutoplay();
      ref.snapToItem(0, true);
    }
  },
  {
    name: "loop",
    getProperties: () => {
      let prop = {
        loop: true,
        loopClonesPerSide: 5
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "InactiveSlide",
    getProperties: () => {
      let prop = {
        inactiveSlideOpacity: 0.4,
        inactiveSlideScale: 0.4,
        inactiveSlideShift: 70
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "itemWidth",
    getProperties: () => {
      let prop = {
        itemWidth: 100
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "layout-stack",
    getProperties: () => {
      let prop = {
        layout: "stack",
        layoutCardOffset: 20,
        key: getRandomInt(15)
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "layout-tinder",
    getProperties: () => {
      let prop = {
        layout: "tinder",
        layoutCardOffset: 20,
        key: getRandomInt(15)
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "layout-defult",
    getProperties: () => {
      let prop = {
        layout: "default",
        layoutCardOffset: -20,
        key: getRandomInt(15)
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "SlideAlignment-center",
    getProperties: () => {
      let prop = {
        activeSlideAlignment: "center"
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "SlideAlignment-start",
    getProperties: () => {
      let prop = {
        activeSlideAlignment: "start"
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "SlideAlignment-end",
    getProperties: () => {
      let prop = {
        activeSlideAlignment: "end"
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "containerCustomStyle",
    getProperties: () => {
      let prop = {
        containerCustomStyle: { marginTop: 20 }
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "Vertical",
    getProperties: () => {
      let prop = {
        itemHeight: 200,
        sliderHeight: 600,
        vertical: true
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  },
  {
    name: "Horizontal",
    getProperties: () => {
      let prop = {
        horizontal: true
      };
      prop = { ...defaultCarouselProps, ...prop };
      return prop;
    },
    action: (ref) => {
      ref.snapToItem(0, true);
    }
  }
];

const App = () => {
  const [snapProps, setSnapProps] = React.useState(defaultCarouselProps);
  const carouselRef = React.useRef();

  const OnButtonPress = (item) => {
    let props = item.getProperties();

    if (item.isToggle) {
      props = toggleValue(item);
    }

    mergeProps(props);

    if (typeof item.action === "function") {
      item.action(carouselRef.current);
    }
  };

  const toggleValue = (prop, value) => {
    let currentValue = snapProps[prop.name];
    let props = {};
    props[prop.name] = !currentValue;
    return props;
  };

  const mergeProps = (newProps) => {
    let _props = { ...snapProps, ...newProps };
    setSnapProps(_props);
  };

  return (
    <View style={styles.mainView}>
      <View styles={styles.leftPanel}>
        {carouselProps.map((item) => {
          return (
            <TouchableHighlight
              isTVSelectable="true"
              underlayColor="#FFBA08"
              style={styles.controlItems}
              onPress={() => OnButtonPress(item)}
            >
              <Text style={styles.text}>{item.name}</Text>
            </TouchableHighlight>
          );
        })}
      </View>
      <View style={styles.contentPanel}>
        <SnapCarouselView {...snapProps} carouselRef={carouselRef} />
      </View>
    </View>
  );
};
const styles = StyleSheet.create({
  mainView: {
    flex: 1,
    flexDirection: "row",
    flexWrap: "wrap",
    alignItems: "flex-start",
    justifyContent: "center"
  },
  leftPanel: {
    width: 200
  },
  contentPanel: {
    flex: 2
  },
  controlItems: {
    margin: 8,
    alignItems: "center",
    justifyContent: "center",
    width: 250,
    height: 37,
    borderColor: "black",
    borderWidth: 2,
    backgroundColor: "#3539"
  },
  text: {
    margin: 2,
    fontSize: 12
  }
});

export default App;
AppRegistry.registerComponent("SimpleViewApp", () => App);
