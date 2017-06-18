# meson project file

project('Parcival', 'cpp',
    version: '0.0.1',
    license: 'Apache v2')

subdir('./src')

local_include = include_directories('src')

add_project_arguments(
    ['-Wall', '-Werror', '-std=c++17', '-O3'],
    include_directories: local_include,
    language: 'cpp')

# ===========================================================================

subdir('./test')

gtest_dep = dependency('gtest')

test_trampoline = executable(
    'test-all',
    test_parsing_files,
    test_main_files,
    dependencies: [gtest_dep],
    include_directories: local_include)

test('test-all', test_trampoline)

