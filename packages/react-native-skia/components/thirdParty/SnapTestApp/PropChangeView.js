import * as React from "react";
import { Text } from "react-native";

const PropChangeView = (props) => {
  return (
    <>
      <Text style={{ fontSize: 30 }}>Props changes</Text>
      <Text
        style={{
          fontSize: 18,
          width: 400,
          height: 250,
          borderColor: "green",
          borderWidth: 1,
          padding: 10
        }}
      >
        {JSON.stringify(props)}
      </Text>
    </>
  );
};

export default PropChangeView;
