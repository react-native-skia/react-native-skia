# Copyright 2019 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from android_chrome_version import GenerateVersionCodes
from android_chrome_version import TranslateVersionCode

EXAMPLE_VERSION_VALUES = {
    'MAJOR': '99',
    'MINOR': '0',
    'BUILD': '4844',
    'PATCH': '0',
}


class _VersionTest(unittest.TestCase):
  """Unittests for the android_chrome_version module.
  """

  def testGenerateVersionCodesAndroidChrome(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(chrome_version_code, '484400000')

  def testGenerateVersionCodesAndroidChromeModern(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    chrome_modern_version_code = output['CHROME_MODERN_VERSION_CODE']

    self.assertEqual(chrome_modern_version_code, '484400010')

  def testGenerateVersionCodesAndroidMonochrome(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    monochrome_version_code = output['MONOCHROME_VERSION_CODE']

    self.assertEqual(monochrome_version_code, '484400020')

  def testGenerateVersionCodesAndroidTrichrome(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    trichrome_version_code = output['TRICHROME_VERSION_CODE']

    self.assertEqual(trichrome_version_code, '484400030')

  def testGenerateVersionCodesAndroidWebviewStable(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    webview_stable_version_code = output['WEBVIEW_STABLE_VERSION_CODE']

    self.assertEqual(webview_stable_version_code, '484400000')

  def testGenerateVersionCodesAndroidWebviewBeta(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    webview_beta_version_code = output['WEBVIEW_BETA_VERSION_CODE']

    self.assertEqual(webview_beta_version_code, '484400010')

  def testGenerateVersionCodesAndroidWebviewDev(self):
    """Assert it gives correct values for standard/example inputs"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    webview_dev_version_code = output['WEBVIEW_DEV_VERSION_CODE']

    self.assertEqual(webview_dev_version_code, '484400020')

  def testGenerateVersionCodesAndroidNextBuild(self):
    """Assert it handles "next" builds correctly"""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=True)

    # Get just a sample of values
    chrome_version_code = output['CHROME_VERSION_CODE']
    monochrome_version_code = output['MONOCHROME_VERSION_CODE']
    webview_stable_version_code = output['WEBVIEW_STABLE_VERSION_CODE']
    webview_beta_version_code = output['WEBVIEW_BETA_VERSION_CODE']

    self.assertEqual(chrome_version_code, '484450000')
    self.assertEqual(monochrome_version_code, '484450020')
    self.assertEqual(webview_stable_version_code, '484450000')
    self.assertEqual(webview_beta_version_code, '484450010')

  def testGenerateVersionCodesAndroidArchArm(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docs in android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)
    arch_chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(arch_chrome_version_code, '484400000')

  def testGenerateVersionCodesAndroidArchX86(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='x86',
                                  is_next_build=False)
    arch_chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(arch_chrome_version_code, '484400001')

  def testGenerateVersionCodesAndroidArchMips(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='mipsel',
                                  is_next_build=False)
    arch_chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(arch_chrome_version_code, '484400002')

  def testGenerateVersionCodesAndroidArchArm64(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm64',
                                  is_next_build=False)
    arch_chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(arch_chrome_version_code, '484400005')

  def testGenerateVersionCodesAndroidArchArm64Variants(self):
    """Assert it handles 64-bit-specific additional version codes correctly.

    Some additional version codes are generated for 64-bit architectures.
    See docstring on android_chrome_version.ARCH64_APK_VARIANTS for more info.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm64',
                                  is_next_build=False)
    arch_monochrome_version_code = output['MONOCHROME_VERSION_CODE']
    arch_monochrome_32_version_code = output['MONOCHROME_32_VERSION_CODE']
    arch_monochrome_32_64_version_code = output['MONOCHROME_32_64_VERSION_CODE']
    arch_monochrome_64_32_version_code = output['MONOCHROME_64_32_VERSION_CODE']
    arch_monochrome_64_version_code = output['MONOCHROME_64_VERSION_CODE']
    arch_trichrome_version_code = output['TRICHROME_VERSION_CODE']
    arch_trichrome_32_version_code = output['TRICHROME_32_VERSION_CODE']
    arch_trichrome_32_64_version_code = output['TRICHROME_32_64_VERSION_CODE']
    arch_trichrome_64_32_version_code = output['TRICHROME_64_32_VERSION_CODE']
    arch_trichrome_64_version_code = output['TRICHROME_64_VERSION_CODE']

    self.assertEqual(arch_monochrome_32_version_code, '484400020')
    self.assertEqual(arch_monochrome_32_64_version_code, '484400023')
    self.assertEqual(arch_monochrome_version_code, '484400023')
    self.assertEqual(arch_monochrome_64_32_version_code, '484400024')
    self.assertEqual(arch_monochrome_64_version_code, '484400025')
    self.assertEqual(arch_trichrome_32_version_code, '484400030')
    self.assertEqual(arch_trichrome_32_64_version_code, '484400033')
    self.assertEqual(arch_trichrome_version_code, '484400033')
    self.assertEqual(arch_trichrome_64_32_version_code, '484400034')
    self.assertEqual(arch_trichrome_64_version_code, '484400035')

  def testGenerateVersionCodesAndroidArchX64(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='x64',
                                  is_next_build=False)
    arch_chrome_version_code = output['CHROME_VERSION_CODE']

    self.assertEqual(arch_chrome_version_code, '484400008')

  def testGenerateVersionCodesAndroidArchX64Variants(self):
    """Assert it handles 64-bit-specific additional version codes correctly.

    Some additional version codes are generated for 64-bit architectures.
    See docstring on android_chrome_version.ARCH64_APK_VARIANTS for more info.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='x64',
                                  is_next_build=False)
    arch_monochrome_32_version_code = output['MONOCHROME_32_VERSION_CODE']
    arch_monochrome_32_64_version_code = output['MONOCHROME_32_64_VERSION_CODE']
    arch_monochrome_version_code = output['MONOCHROME_VERSION_CODE']
    arch_monochrome_64_32_version_code = output['MONOCHROME_64_32_VERSION_CODE']
    arch_monochrome_64_version_code = output['MONOCHROME_64_VERSION_CODE']
    arch_trichrome_32_version_code = output['TRICHROME_32_VERSION_CODE']
    arch_trichrome_32_64_version_code = output['TRICHROME_32_64_VERSION_CODE']
    arch_trichrome_version_code = output['TRICHROME_VERSION_CODE']
    arch_trichrome_64_32_version_code = output['TRICHROME_64_32_VERSION_CODE']
    arch_trichrome_64_version_code = output['TRICHROME_64_VERSION_CODE']

    self.assertEqual(arch_monochrome_32_version_code, '484400021')
    self.assertEqual(arch_monochrome_32_64_version_code, '484400026')
    self.assertEqual(arch_monochrome_version_code, '484400026')
    self.assertEqual(arch_monochrome_64_32_version_code, '484400027')
    self.assertEqual(arch_monochrome_64_version_code, '484400028')
    self.assertEqual(arch_trichrome_32_version_code, '484400031')
    self.assertEqual(arch_trichrome_32_64_version_code, '484400036')
    self.assertEqual(arch_trichrome_version_code, '484400036')
    self.assertEqual(arch_trichrome_64_32_version_code, '484400037')
    self.assertEqual(arch_trichrome_64_version_code, '484400038')

  def testGenerateVersionCodesAndroidArchOrderArm(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.

    Test arm-related values.
    """
    arm_output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                      arch='arm',
                                      is_next_build=False)
    arm64_output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                        arch='arm64',
                                        is_next_build=False)

    arm_chrome_version_code = arm_output['CHROME_VERSION_CODE']
    arm64_chrome_version_code = arm64_output['CHROME_VERSION_CODE']

    self.assertLess(arm_chrome_version_code, arm64_chrome_version_code)

  def testGenerateVersionCodesAndroidArchOrderX86(self):
    """Assert it handles different architectures correctly.

    Version codes for different builds need to be distinct and maintain a
    certain ordering.
    See docstring on android_chrome_version._ABIS_TO_BIT_MASK for
    reasoning.

    Test x86-related values.
    """
    x86_output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                      arch='x86',
                                      is_next_build=False)
    x64_output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                      arch='x64',
                                      is_next_build=False)

    x86_chrome_version_code = x86_output['CHROME_VERSION_CODE']
    x64_chrome_version_code = x64_output['CHROME_VERSION_CODE']

    self.assertLess(x86_chrome_version_code, x64_chrome_version_code)

  def testGenerateVersionCodesAndroidWebviewChannelOrderBeta(self):
    """Assert webview beta channel is higher than stable.

    The channel-specific version codes for standalone webview needs to follow
    the order stable < beta < dev.

    This allows that if a user opts into beta track, they will always have the
    beta apk, including any finch experiments targeted at beta users, even when
    beta and stable channels are otherwise on the same version.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    webview_stable_version_code = output['WEBVIEW_STABLE_VERSION_CODE']
    webview_beta_version_code = output['WEBVIEW_BETA_VERSION_CODE']

    self.assertGreater(webview_beta_version_code, webview_stable_version_code)

  def testGenerateVersionCodesAndroidWebviewChannelOrderDev(self):
    """Assert webview dev channel is higher than beta.

    The channel-specific version codes for standalone webview needs to follow
    the order stable < beta < dev.

    This allows that if a user opts into dev track, they will always have the
    dev apk, including any finch experiments targeted at dev users, even when
    dev and beta channels are otherwise on the same version.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    webview_beta_version_code = output['WEBVIEW_BETA_VERSION_CODE']
    webview_dev_version_code = output['WEBVIEW_DEV_VERSION_CODE']

    self.assertGreater(webview_dev_version_code, webview_beta_version_code)

  def testGenerateVersionCodesTrichromeChannelOrderBeta(self):
    """Assert Trichrome beta channel is higher than stable.

    When Trichrome channels are compiled to use the stable channel's package
    name, their version codes need to follow the order stable < beta.

    This allows that if a user opts into beta track, they will always have the
    beta apk, including any finch experiments targeted at beta users, even when
    beta and stable channels are otherwise on the same version.
    """
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    trichrome_stable_version_code = output['TRICHROME_VERSION_CODE']
    trichrome_beta_version_code = output['TRICHROME_BETA_VERSION_CODE']

    self.assertGreater(trichrome_beta_version_code,
                       trichrome_stable_version_code)


class _VersionCodeTest(unittest.TestCase):
  def testGenerateThenTranslate(self):
    """Assert it gives correct values for a version code that we generated."""
    output = GenerateVersionCodes(EXAMPLE_VERSION_VALUES,
                                  arch='arm',
                                  is_next_build=False)

    version_code = output['MONOCHROME_VERSION_CODE']

    build, patch, package, abi, is_next_build = TranslateVersionCode(
        version_code)
    self.assertEqual(build, int(EXAMPLE_VERSION_VALUES['BUILD']))
    self.assertEqual(patch, int(EXAMPLE_VERSION_VALUES['PATCH']))
    self.assertEqual(package, 'MONOCHROME')
    self.assertEqual(abi, 'arm')
    self.assertEqual(is_next_build, False)

  def testPre3992Translate(self):
    """Test for an old build when the abi and apk bits were swapped."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '378100010')
    self.assertEqual(build, 3781)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME')
    self.assertEqual(abi, 'x86')
    self.assertEqual(is_next_build, False)

  def testNextBuildTranslate(self):
    """Test for a build with next."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499961210')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 112)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm')
    self.assertEqual(is_next_build, True)

  def testPre4844NextBuildTranslate(self):
    """Test for a build with next when we added 50 to version code."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '400011260')
    self.assertEqual(build, 4000)
    self.assertEqual(patch, 112)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm')
    self.assertEqual(is_next_build, True)

  def testPre3992NextBuildTranslate(self):
    """Test for a build with next when we added 5 to version code."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '300011206')
    self.assertEqual(build, 3000)
    self.assertEqual(patch, 112)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm')
    self.assertEqual(is_next_build, True)

  def testArm_64BuildTranslate(self):
    """Test for a build with arm_64."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499911215')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 112)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm_64')
    self.assertEqual(is_next_build, False)

  def testArm_32_64Translate(self):
    """Test for a build with arm_32_64."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900013')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm_32_64')
    self.assertEqual(is_next_build, False)

  def testArm_64_32Translate(self):
    """Test for a build with Trichrome and arm_64_32."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900014')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'arm_64_32')
    self.assertEqual(is_next_build, False)

  def testX86_64Translate(self):
    """Test for a build with x86_64."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900018')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'x86_64')
    self.assertEqual(is_next_build, False)

  def testX86_32_64Translate(self):
    """Test for a build with x86_32_64."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900016')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'x86_32_64')
    self.assertEqual(is_next_build, False)

  def testX86_64_32Translate(self):
    """Test for a build with x86_64_32."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900017')
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'CHROME_MODERN')
    self.assertEqual(abi, 'x86_64_32')
    self.assertEqual(is_next_build, False)

  def testWebviewTranslate(self):
    """Test for a build with Webview."""
    build, patch, package, abi, is_next_build = TranslateVersionCode(
        '499900000', is_webview=True)
    self.assertEqual(build, 4999)
    self.assertEqual(patch, 0)
    self.assertEqual(package, 'WEBVIEW_STABLE')
    self.assertEqual(abi, 'arm')
    self.assertEqual(is_next_build, False)


if __name__ == '__main__':
  unittest.main()
