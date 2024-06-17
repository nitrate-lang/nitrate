# This script processes a manifest.json file and data sources to generate Google Test cases for the libquixcc test suite.

import sys
import os
import json

with open(sys.argv[1], 'r') as f:
    content = f.read()

headerinc = '/* PYTHON_INSERT_INCLUDE */'

loc = os.path.abspath('../../../libquixcc/include/quixcc.h')
if not os.path.exists(loc):
    raise Exception(f'{loc} does not exist')

content = content.replace(headerinc, f'#include "{loc}"')

# remove everything after the string `/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */`
content = content.split('/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */')[
    0] + '/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */\n\n'


def generate_test_cases_for_manifest(manifest_file):
    global content

    with open(manifest_file, 'r') as f:
        test_cases_manifest = json.load(f)

    if 'version' not in test_cases_manifest:
        raise Exception('version not found in manifest.json')

    if test_cases_manifest['version'] != '1.0':
        raise Exception('Unsupported manifest version')

    test_suite_name = 'libquixcc'

    def make_test_name(name):
        tmp = (''.join([c if c.isalnum() else '_' for c in name])).lower()
        return '_'.join([x for x in tmp.split('_') if x])

    def rectify_path(path) -> str:
        return os.path.abspath('../../../libquixcc/test/data/inputs/' + path)

    def assert_exists(file):
        if not os.path.exists(file):
            raise Exception(f'{file} does not exist')

    def generate_output_name(test_name):
        return f'/tmp/{test_suite_name}_{test_name}.tmp.out'

    for test_case in test_cases_manifest['test_cases']:
        name = make_test_name(test_case['name'])
        description = test_case['note']
        source_file = rectify_path(test_case['input'])
        build_mode = test_case['build']
        expected_output = test_case['expected']
        output_name = generate_output_name(name)
        flags = []
        if 'cflags' in test_case:
            flags += test_case['cflags']

        assert_exists(source_file)

        content += f"TEST({test_suite_name}, {name}) {{\n"
        content += f"    ASSERT_TRUE(quixcc_init());\n"
        content += f"    const char* source_file = \"{source_file}\";\n"
        content += f"    const char* output_file = \"{output_name}\";\n"
        if build_mode == 'asm-sha1' or build_mode == 'obj-sha1':
            content += f"    uint8_t sha1[20];\n"
            content += f"    static const uint8_t expected[20] = {{"
            for i in range(0, len(expected_output), 2):
                content += f"0x{expected_output[i:i+2]}"
                if i + 2 < len(expected_output):
                    content += ','
            content += f"}};\n"
        content += f"    FILE *output;\n"

        content += f"    if ((output = fopen(output_file, \"w+\")) == NULL) {{ FAIL() << \"Could not open file: \" << output_file; }}\n"

        if build_mode == 'asm-sha1':
            content += f"""    ASSERT_TRUE(quixcc::CompilerBuilder().add_source(source_file).set_output(output).opt(\"-S\")"""
            for flag in flags:
                content += f".opt(\"{flag}\")"
            content += f".build().run().ok());\n"
        elif build_mode == 'obj-sha1':
            content += f"""    ASSERT_TRUE(quixcc::CompilerBuilder().add_source(source_file).set_output(output).opt(\"-c\")"""
            for flag in flags:
                content += f".opt(\"{flag}\")"
            content += f".build().run().ok());\n"
        elif build_mode == 'asm':
            content += f"""    ASSERT_EQ(quixcc::CompilerBuilder().add_source(source_file).set_output(output).opt(\"-S\")"""
            for flag in flags:
                content += f".opt(\"{flag}\")"
            content += f".build().run().ok(), {expected_output});\n"
        elif build_mode == 'obj':
            content += f"""    ASSERT_EQ(quixcc::CompilerBuilder().add_source(source_file).set_output(output).opt(\"-c\")"""
            for flag in flags:
                content += f".opt(\"{flag}\")"
            content += f".build().run().ok(), {expected_output});\n"
        else:
            raise Exception(f'Unknown build mode {build_mode}')

        if build_mode == 'asm-sha1' or build_mode == 'obj-sha1':
            content += f"    sha1_file_ignstr(output,sha1);\n"
            content += f"    bool match;EXPECT_TRUE(match = (memcmp(sha1, expected, 20) == 0));\n"
            if build_mode == 'asm-sha1':
                content += f"    if (!match) {{\n"
                content += f"        char line[256];\n"
                content += f"        while (fgets(line, sizeof(line), output)) {{\n"
                content += f"            std::cout << line;\n"
                content += f"        }}\n"
                content += f"    }}\n"

        content += f"    fclose(output);\n"
        content += f"    remove(output_file);\n"
        content += f"}}\n"


def generate_test_suite():
    with open(sys.argv[2] + '/manifest.json', 'r') as f:
        root_manifest = json.load(f)

    if root_manifest['version'] != '1.0':
        raise Exception('Unsupported manifest version')

    for file in root_manifest['imports']:
        generate_test_cases_for_manifest(
            sys.argv[2] + '/inputs/' + file + '/manifest.json')


generate_test_suite()

with open(sys.argv[3], 'w') as f:
    f.write(content)
