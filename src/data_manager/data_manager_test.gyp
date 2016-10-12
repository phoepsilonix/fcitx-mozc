# Copyright 2010-2016, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{
  'variables': {
    'relative_dir': 'data_manager',
    'gen_out_dir': '<(SHARED_INTERMEDIATE_DIR)/<(relative_dir)',
  },
  'targets': [
    {
      'target_name': 'data_manager_test_base',
      'type': 'static_library',
      'sources': [
        'data_manager_test_base.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../converter/converter_base.gyp:connector',
        '../converter/converter_base.gyp:segmenter',
        '../dictionary/dictionary_base.gyp:pos_matcher',
        '../prediction/prediction_base.gyp:suggestion_filter',
        '../testing/testing.gyp:testing',
        'data_manager.gyp:connection_file_reader',
      ],
    },
    {
      'target_name': 'dataset_writer_test',
      'type': 'executable',
      'toolsets': [ 'target' ],
      'sources': [
        'dataset_writer_test.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../testing/testing.gyp:gtest_main',
        '../testing/testing.gyp:mozctest',
        '../testing/testing.gyp:testing',
        'data_manager_base.gyp:dataset_proto',
        'data_manager_base.gyp:dataset_writer',
      ],
    },
    {
      'target_name': 'dataset_reader_test',
      'type': 'executable',
      'toolsets': [ 'target' ],
      'sources': [
        'dataset_reader_test.cc',
      ],
      'dependencies': [
        '../base/base.gyp:base',
        '../testing/testing.gyp:gtest_main',
        '../testing/testing.gyp:testing',
        'data_manager_base.gyp:dataset_proto',
        'data_manager_base.gyp:dataset_reader',
        'data_manager_base.gyp:dataset_writer',
      ],
    },
    {
      'target_name': 'serialized_dictionary_test',
      'type': 'executable',
      'sources': ['serialized_dictionary_test.cc'],
      'dependencies': [
        '../base/base.gyp:serialized_string_array',
        '../testing/testing.gyp:gtest_main',
        'data_manager_base.gyp:serialized_dictionary',
      ],
    },
  ],
}
