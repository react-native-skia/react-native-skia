#!/usr/bin/env python3
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

sys.path += [os.path.dirname(os.path.dirname(__file__))]

from style_variable_generator.base_generator import BaseGenerator, VariableType, Modes
import unittest


class BaseGeneratorTest(unittest.TestCase):
    def setUp(self):
        self.generator = BaseGenerator()

    def ResolveOpacity(self, name, mode=Modes.LIGHT):
        opacity_model = self.generator.model[VariableType.OPACITY]
        opacity = opacity_model.Resolve(name, mode)
        return opacity_model.ResolveOpacity(opacity, mode).a

    def ResolveRGBA(self, name, mode=Modes.LIGHT):
        return repr(
            self.generator.model[VariableType.COLOR].ResolveToRGBA(name, mode))

    def testMissingColor(self):
        # google_grey_900 is missing.
        self.generator.AddJSONToModel('''
{
  colors: {
    cros_default_text_color: {
      light: "$google_grey_900",
      dark: "rgb(255, 255, 255)",
    },
  },
}
        ''')
        self.assertRaises(ValueError, self.generator.Validate)

        # Add google_grey_900.
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: "rgb(255, 255, 255)",
  }
}
        ''')
        self.generator.Validate()

    def testMissingDefaultModeColor(self):
        # google_grey_900 is missing in the default mode (light).
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: { dark: "rgb(255, 255, 255)", },
  }
}
        ''')
        self.assertRaises(ValueError, self.generator.Validate)

    def testDuplicateKeys(self):
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: { light: "rgb(255, 255, 255)", },
  }
}
        ''')
        self.generator.Validate()

        # Add google_grey_900's dark mode as if in a separate file. This counts
        # as a redefinition/conflict and causes an error.
        self.assertRaises(
            ValueError, self.generator.AddJSONToModel, '''
{
  colors: {
    google_grey_900: { dark: "rgb(255, 255, 255)", }
  }
}
        ''')

    def testBadNames(self):
        # Add a bad color name.
        self.assertRaises(
            ValueError, self.generator.AddJSONToModel, '''
{
  colors: {
    Google+grey: { dark: "rgb(255, 255, 255)", }
  }
}
        ''')
        # Add a bad opacity name.
        self.assertRaises(
            ValueError, self.generator.AddJSONToModel, '''
{
  opacities: {
    disabled_things: 0.4,
  }
}
        ''')

    def testSimpleOpacity(self):
        # Reference a missing opacity.
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: "rgba(255, 255, 255, $disabled_opacity)",
  },
  opacities: {
    disabled_opacity: 0.5,
  },
}
        ''')

        self.assertEqual(self.ResolveRGBA('google_grey_900'),
                         'rgba(255, 255, 255, 0.5)')

        self.generator.Validate()

    def testReferenceOpacity(self):
        # Add a reference opacity.
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: "rgba(255, 255, 255, $disabled_opacity)",
  },
  opacities: {
    disabled_opacity: "$second_opacity",
    second_opacity: { "light": "$another_opacity", "dark": 1 },
    another_opacity: 0.5,
  },
}
        ''')
        self.assertEqual(self.ResolveRGBA('google_grey_900'),
                         'rgba(255, 255, 255, 0.5)')
        self.assertEqual(self.ResolveOpacity('disabled_opacity'), 0.5)
        self.assertEqual(self.ResolveOpacity('disabled_opacity', Modes.DARK), 1)

    def testMissingOpacity(self):
        # Reference a missing opacity.
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: "rgba(255, 255, 255, $missing_opacity)",
  },
  opacities: {
    disabled_opacity: 0.5,
  },
}
        ''')
        self.assertRaises(ValueError, self.generator.Validate)

        self.generator.AddJSONToModel('''
{
  opacities: {
    missing_opacity: 0.5,
  },
}
        ''')
        self.generator.Validate()

    def testSelfReferenceColor(self):
        self.generator.AddJSONToModel('''
{
  colors: {
    google_grey_900: "$google_grey_900",
  }
}
        ''')
        self.assertRaises(ValueError, self.generator.Validate)

    def testSelfReferenceOpacity(self):
        self.generator.AddJSONToModel('''
{
  opacities: {
    some_opacity: "$some_opacity",
  }
}
        ''')
        self.assertRaises(ValueError, self.generator.Validate)


if __name__ == '__main__':
    unittest.main()
