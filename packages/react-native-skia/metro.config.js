const path = require('path');

module.exports = {
  transformer: {
    getTransformOptions: async () => ({
      transform: {
        experimentalImportSupport: false,
        inlineRequires: true,
      },
    }),
  },

  watchFolders: [
    path.resolve(__dirname, '..', '..', 'node_modules'),
  ],
};
