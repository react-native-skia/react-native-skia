const ViewNames = [
  {
    name: "SVG Elements",
    module: require("./SvgElements/App").default
  },
  {
    name: "SVG Props",
    module: require("./SvgFeatures/App").default
  },
  {
    name: "Dynamic Property Update",
    module: require("./DynamicPropertUpdate").default
  },
  {
    name: "Simple App",
    module: require("./SimpleViewApp").default
  },  
];

export default ViewNames;

export const AppName = 'React Native Skia SVG Demo App';
