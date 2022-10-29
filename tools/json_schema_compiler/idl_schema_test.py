#!/usr/bin/env python3
# Copyright 2012 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import idl_schema
import unittest

from json_parse import OrderedDict

def getFunction(schema, name):
  for item in schema['functions']:
    if item['name'] == name:
      return item
  raise KeyError('Missing function %s' % name)


def getParams(schema, name):
  function = getFunction(schema, name)
  return function['parameters']


def getReturns(schema, name):
  function = getFunction(schema, name)
  return function['returns']


def getType(schema, id):
  for item in schema['types']:
    if item['id'] == id:
      return item


class IdlSchemaTest(unittest.TestCase):
  def setUp(self):
    loaded = idl_schema.Load('test/idl_basics.idl')
    self.assertEqual(1, len(loaded))
    self.assertEqual('idl_basics', loaded[0]['namespace'])
    self.idl_basics = loaded[0]
    self.maxDiff = None

  def testSimpleCallbacks(self):
    schema = self.idl_basics
    expected = [{'type': 'function', 'name': 'cb', 'parameters':[]}]
    self.assertEqual(expected, getParams(schema, 'function4'))

    expected = [{'type': 'function', 'name': 'cb',
                 'parameters':[{'name': 'x', 'type': 'integer'}]}]
    self.assertEqual(expected, getParams(schema, 'function5'))

    expected = [{'type': 'function', 'name': 'cb',
                 'parameters':[{'name': 'arg', '$ref': 'MyType1'}]}]
    self.assertEqual(expected, getParams(schema, 'function6'))

  def testCallbackWithArrayArgument(self):
    schema = self.idl_basics
    expected = [{'type': 'function', 'name': 'cb',
                 'parameters':[{'name': 'arg', 'type': 'array',
                                'items':{'$ref': 'MyType2'}}]}]
    self.assertEqual(expected, getParams(schema, 'function12'))


  def testArrayOfCallbacks(self):
    schema = idl_schema.Load('test/idl_function_types.idl')[0]
    expected = [{'type': 'array', 'name': 'callbacks',
                 'items':{'type': 'function', 'name': 'MyCallback',
                          'parameters':[{'type': 'integer', 'name': 'x'}]}}]
    self.assertEqual(expected, getParams(schema, 'whatever'))

  def testLegalValues(self):
    self.assertEqual({
        'x': {'name': 'x', 'type': 'integer', 'enum': [1,2],
              'description': 'This comment tests "double-quotes".',
              'jsexterns': None},
        'y': {'name': 'y', 'type': 'string'},
        'z': {'name': 'z', 'type': 'string'},
        'a': {'name': 'a', 'type': 'string'},
        'b': {'name': 'b', 'type': 'string'},
        'c': {'name': 'c', 'type': 'string'}},
      getType(self.idl_basics, 'MyType1')['properties'])

  def testMemberOrdering(self):
    self.assertEqual(
        ['x', 'y', 'z', 'a', 'b', 'c'],
        list(getType(self.idl_basics, 'MyType1')['properties'].keys()))

  def testEnum(self):
    schema = self.idl_basics
    expected = {'enum': [{'name': 'name1', 'description': 'comment1'},
                         {'name': 'name2'}],
                'description': 'Enum description',
                'type': 'string', 'id': 'EnumType'}
    self.assertEqual(expected, getType(schema, expected['id']))

    expected = [{'name': 'type', '$ref': 'EnumType'},
                {'type': 'function', 'name': 'cb',
                  'parameters':[{'name': 'type', '$ref': 'EnumType'}]}]
    self.assertEqual(expected, getParams(schema, 'function13'))

    expected = [{'items': {'$ref': 'EnumType'}, 'name': 'types',
                 'type': 'array'}]
    self.assertEqual(expected, getParams(schema, 'function14'))

  def testScopedArguments(self):
    schema = self.idl_basics
    expected = [{'name': 'value', '$ref': 'idl_other_namespace.SomeType'}]
    self.assertEqual(expected, getParams(schema, 'function20'))

    expected = [{'items': {'$ref': 'idl_other_namespace.SomeType'},
                 'name': 'values',
                 'type': 'array'}]
    self.assertEqual(expected, getParams(schema, 'function21'))

    expected = [{'name': 'value',
                 '$ref': 'idl_other_namespace.sub_namespace.AnotherType'}]
    self.assertEqual(expected, getParams(schema, 'function22'))

    expected = [{'items': {'$ref': 'idl_other_namespace.sub_namespace.'
                                   'AnotherType'},
                 'name': 'values',
                 'type': 'array'}]
    self.assertEqual(expected, getParams(schema, 'function23'))

  def testNoCompile(self):
    schema = self.idl_basics
    func = getFunction(schema, 'function15')
    self.assertTrue(func is not None)
    self.assertTrue(func['nocompile'])

  def testNoDocOnEnum(self):
    schema = self.idl_basics
    enum_with_nodoc = getType(schema, 'EnumTypeWithNoDoc')
    self.assertTrue(enum_with_nodoc is not None)
    self.assertTrue(enum_with_nodoc['nodoc'])

  def testNoDocOnEnumValue(self):
    schema = self.idl_basics
    expected = {
        'enum': [{
            'name': 'name1'
        }, {
            'name': 'name2',
            'nodoc': True,
            'description': 'comment2'
        }, {
            'name': 'name3',
            'description': 'comment3'
        }],
        'type': 'string',
        'id': 'EnumTypeWithNoDocValue',
        'description': ''
    }
    self.assertEqual(expected, getType(schema, expected['id']))

  def testInternalNamespace(self):
    idl_basics  = self.idl_basics
    self.assertEqual('idl_basics', idl_basics['namespace'])
    self.assertTrue(idl_basics['internal'])
    self.assertFalse(idl_basics['nodoc'])

  def testReturnTypes(self):
    schema = self.idl_basics
    self.assertEqual({'name': 'function24', 'type': 'integer'},
                      getReturns(schema, 'function24'))
    self.assertEqual({'name': 'function25', '$ref': 'MyType1',
                       'optional': True},
                      getReturns(schema, 'function25'))
    self.assertEqual({'name': 'function26', 'type': 'array',
                       'items': {'$ref': 'MyType1'}},
                      getReturns(schema, 'function26'))
    self.assertEqual({'name': 'function27', '$ref': 'EnumType',
                       'optional': True},
                      getReturns(schema, 'function27'))
    self.assertEqual({'name': 'function28', 'type': 'array',
                       'items': {'$ref': 'EnumType'}},
                      getReturns(schema, 'function28'))
    self.assertEqual({'name': 'function29', '$ref':
                       'idl_other_namespace.SomeType',
                       'optional': True},
                      getReturns(schema, 'function29'))
    self.assertEqual({'name': 'function30', 'type': 'array',
                       'items': {'$ref': 'idl_other_namespace.SomeType'}},
                      getReturns(schema, 'function30'))

  def testChromeOSPlatformsNamespace(self):
    schema = idl_schema.Load('test/idl_namespace_chromeos.idl')[0]
    self.assertEqual('idl_namespace_chromeos', schema['namespace'])
    expected = ['chromeos']
    self.assertEqual(expected, schema['platforms'])

  def testAllPlatformsNamespace(self):
    schema = idl_schema.Load('test/idl_namespace_all_platforms.idl')[0]
    self.assertEqual('idl_namespace_all_platforms', schema['namespace'])
    expected = ['chromeos', 'fuchsia', 'linux', 'mac', 'win']
    self.assertEqual(expected, schema['platforms'])

  def testNonSpecificPlatformsNamespace(self):
    schema = idl_schema.Load('test/idl_namespace_non_specific_platforms.idl')[0]
    self.assertEqual('idl_namespace_non_specific_platforms',
                      schema['namespace'])
    expected = None
    self.assertEqual(expected, schema['platforms'])

  def testGenerateErrorMessages(self):
    schema = idl_schema.Load('test/idl_generate_error_messages.idl')[0]
    self.assertEqual('idl_generate_error_messages', schema['namespace'])
    self.assertTrue(schema['compiler_options'].get('generate_error_messages',
                    False))

    schema = idl_schema.Load('test/idl_basics.idl')[0]
    self.assertEqual('idl_basics', schema['namespace'])
    self.assertFalse(schema['compiler_options'].get('generate_error_messages',
                     False))

  def testSpecificImplementNamespace(self):
    schema = idl_schema.Load('test/idl_namespace_specific_implement.idl')[0]
    self.assertEqual('idl_namespace_specific_implement',
                      schema['namespace'])
    expected = 'idl_namespace_specific_implement.idl'
    self.assertEqual(expected, schema['compiler_options']['implemented_in'])

  def testSpecificImplementOnChromeOSNamespace(self):
    schema = idl_schema.Load(
        'test/idl_namespace_specific_implement_chromeos.idl')[0]
    self.assertEqual('idl_namespace_specific_implement_chromeos',
                      schema['namespace'])
    expected_implemented_path = 'idl_namespace_specific_implement_chromeos.idl'
    expected_platform = ['chromeos']
    self.assertEqual(expected_implemented_path,
                      schema['compiler_options']['implemented_in'])
    self.assertEqual(expected_platform, schema['platforms'])

  def testCallbackComment(self):
    schema = self.idl_basics
    self.assertEqual('A comment on a callback.',
                      getParams(schema, 'function16')[0]['description'])
    self.assertEqual(
        'A parameter.',
        getParams(schema, 'function16')[0]['parameters'][0]['description'])
    self.assertEqual(
        'Just a parameter comment, with no comment on the callback.',
        getParams(schema, 'function17')[0]['parameters'][0]['description'])
    self.assertEqual(
        'Override callback comment.',
        getParams(schema, 'function18')[0]['description'])

  def testFunctionComment(self):
    schema = self.idl_basics
    func = getFunction(schema, 'function3')
    self.assertEqual(('This comment should appear in the documentation, '
                       'despite occupying multiple lines.'),
                      func['description'])
    self.assertEqual(
        [{'description': ('So should this comment about the argument. '
                          '<em>HTML</em> is fine too.'),
          'name': 'arg',
          '$ref': 'MyType1'}],
        func['parameters'])
    func = getFunction(schema, 'function4')
    self.assertEqual(
        '<p>This tests if "double-quotes" are escaped correctly.</p>'
        '<p>It also tests a comment with two newlines.</p>',
        func['description'])

  def testReservedWords(self):
    schema = idl_schema.Load('test/idl_reserved_words.idl')[0]

    foo_type = getType(schema, 'Foo')
    self.assertEqual([{'name': 'float'}, {'name': 'DOMString'}],
                      foo_type['enum'])

    enum_type = getType(schema, 'enum')
    self.assertEqual([{'name': 'callback'}, {'name': 'namespace'}],
                      enum_type['enum'])

    dictionary = getType(schema, 'dictionary')
    self.assertEqual('integer', dictionary['properties']['long']['type'])

    mytype = getType(schema, 'MyType')
    self.assertEqual('string', mytype['properties']['interface']['type'])

    params = getParams(schema, 'static')
    self.assertEqual('Foo', params[0]['$ref'])
    self.assertEqual('enum', params[1]['$ref'])

  def testObjectTypes(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]

    foo_type = getType(schema, 'FooType')
    self.assertEqual('object', foo_type['type'])
    self.assertEqual('integer', foo_type['properties']['x']['type'])
    self.assertEqual('object', foo_type['properties']['y']['type'])
    self.assertEqual(
        'any',
        foo_type['properties']['y']['additionalProperties']['type'])
    self.assertEqual('object', foo_type['properties']['z']['type'])
    self.assertEqual(
        'any',
        foo_type['properties']['z']['additionalProperties']['type'])
    self.assertEqual('Window', foo_type['properties']['z']['isInstanceOf'])

    bar_type = getType(schema, 'BarType')
    self.assertEqual('object', bar_type['type'])
    self.assertEqual('any', bar_type['properties']['x']['type'])

  def testObjectTypesInFunctions(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]

    params = getParams(schema, 'objectFunction1')
    self.assertEqual('object', params[0]['type'])
    self.assertEqual('any', params[0]['additionalProperties']['type'])
    self.assertEqual('ImageData', params[0]['isInstanceOf'])

    params = getParams(schema, 'objectFunction2')
    self.assertEqual('any', params[0]['type'])

  def testObjectTypesWithOptionalFields(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]

    baz_type = getType(schema, 'BazType')
    self.assertEqual(True, baz_type['properties']['x']['optional'])
    self.assertEqual('integer', baz_type['properties']['x']['type'])
    self.assertEqual(True, baz_type['properties']['foo']['optional'])
    self.assertEqual('FooType', baz_type['properties']['foo']['$ref'])

  def testObjectTypesWithUnions(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]

    union_type = getType(schema, 'UnionType')
    expected = {
                 'type': 'object',
                 'id': 'UnionType',
                 'properties': {
                   'x': {
                     'name': 'x',
                     'optional': True,
                     'choices': [
                       {'type': 'integer'},
                       {'$ref': 'FooType'},
                     ]
                   },
                   'y': {
                     'name': 'y',
                     'choices': [
                       {'type': 'string'},
                       {'type': 'object',
                        'additionalProperties': {'type': 'any'}}
                     ]
                   },
                   'z': {
                     'name': 'z',
                     'choices': [
                       {'type': 'object', 'isInstanceOf': 'ImageData',
                        'additionalProperties': {'type': 'any'}},
                       {'type': 'integer'}
                     ]
                   }
                 },
               }

    self.assertEqual(expected, union_type)

  def testUnionsWithModifiers(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]

    union_type = getType(schema, 'ModifiedUnionType')
    expected = {
                 'type': 'object',
                 'id': 'ModifiedUnionType',
                 'properties': {
                   'x': {
                     'name': 'x',
                     'nodoc': True,
                     'choices': [
                       {'type': 'integer'},
                       {'type': 'string'}
                     ]
                   }
                 }
               }

    self.assertEqual(expected, union_type)

  def testSerializableFunctionType(self):
    schema = idl_schema.Load('test/idl_object_types.idl')[0]
    object_type = getType(schema, 'SerializableFunctionObject')
    expected = {
                 'type': 'object',
                 'id': 'SerializableFunctionObject',
                 'properties': {
                   'func': {
                     'name': 'func',
                     'serializableFunction': True,
                     'type': 'function',
                     'parameters': []
                   }
                 }
               }
    self.assertEqual(expected, object_type)

  def testUnionsWithFunctions(self):
    schema = idl_schema.Load('test/idl_function_types.idl')[0]

    union_params = getParams(schema, 'union_params')
    expected = [{
                 'name': 'x',
                 'choices': [
                   {'type': 'integer'},
                   {'type': 'string'}
                 ]
               }]

    self.assertEqual(expected, union_params)

  def testUnionsWithCallbacks(self):
    schema = idl_schema.Load('test/idl_function_types.idl')[0]

    blah_params = getParams(schema, 'blah')
    expected = [{
                 'type': 'function', 'name': 'callback', 'parameters': [{
                   'name': 'x',
                   'choices': [
                     {'type': 'integer'},
                     {'type': 'string'}
                   ]}
                 ]
               }]
    self.assertEqual(expected, blah_params)

    badabish_params = getParams(schema, 'badabish')
    expected = [{
                 'type': 'function', 'name': 'callback', 'parameters': [{
                   'name': 'x', 'optional': True, 'choices': [
                     {'type': 'integer'},
                     {'type': 'string'}
                   ]
                 }]
               }]

    self.assertEqual(expected, badabish_params)

  def testFunctionWithPromise(self):
    schema = idl_schema.Load('test/idl_function_types.idl')[0]

    promise_function = getFunction(schema, 'promise_supporting')
    expected = OrderedDict([
        ('parameters', []),
        ('returns_async', {
            'name': 'callback',
            'parameters': [{'name': 'x', 'type': 'integer'}]
        }),
        ('name', 'promise_supporting'),
        ('type', 'function')
    ])
    self.assertEqual(expected, promise_function)

  def testFunctionWithPromiseAndParams(self):
    schema = idl_schema.Load('test/idl_function_types.idl')[0]

    promise_function = getFunction(schema, 'promise_supporting_with_params')
    expected = OrderedDict([
        ('parameters', [
            {
               'name': 'z',
               'type': 'integer'
            }, {
                'name':'y',
                'choices': [{'type': 'integer'}, {'type': 'string'}]
            }
        ]),
        ('returns_async', {
            'name': 'callback',
            'parameters': [{'name': 'x', 'type': 'integer'}]
        }),
        ('name', 'promise_supporting_with_params'),
        ('type', 'function')
    ])
    self.assertEqual(expected, promise_function)

  def testProperties(self):
    schema = idl_schema.Load('test/idl_properties.idl')[0]
    self.assertEqual(OrderedDict([
      ('first', OrderedDict([
        ('description', 'Integer property.'),
        ('jsexterns', None),
        ('type', 'integer'),
        ('value', 42),
      ])),
      ('second', OrderedDict([
        ('description', 'Double property.'),
        ('jsexterns', None),
        ('type', 'number'),
        ('value', 42.1),
      ])),
      ('third', OrderedDict([
        ('description', 'String property.'),
        ('jsexterns', None),
        ('type', 'string'),
        ('value', 'hello world'),
      ])),
      ('fourth', OrderedDict([
        ('description', 'Unvalued property.'),
        ('jsexterns', None),
        ('type', 'integer'),
      ])),
    ]), schema.get('properties'))

  def testManifestKeys(self):
    schema = self.idl_basics
    self.assertEqual(
        OrderedDict([('key_str',
                      OrderedDict([('description', 'String manifest key.'),
                                   ('jsexterns', None), ('name', 'key_str'),
                                   ('type', 'string')])),
                     ('key_ref',
                      OrderedDict([('name', 'key_ref'),
                                   ('$ref', 'MyType2')]))]),
        schema.get('manifest_keys'))

  def testNoManifestKeys(self):
    schema = idl_schema.Load('test/idl_properties.idl')[0]
    self.assertIsNone(schema.get('manifest_keys'))


if __name__ == '__main__':
  unittest.main()
