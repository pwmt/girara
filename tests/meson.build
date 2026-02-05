test_dependencies = [
  declare_dependency(link_with: girara),
]

include_directories += [ include_directories('../girara') ]

xdg_test_helper = executable('xdg_test_helper', 'xdg_test_helper.c',
  dependencies: [ glib, declare_dependency(link_with: girara) ],
  include_directories: include_directories,
  c_args: defines + flags
)

datastructures = executable('test_datastructures', files('test_datastructures.c'),
  dependencies: build_dependencies + test_dependencies,
  include_directories: include_directories,
  c_args: defines + flags
)
test('datastructures', datastructures,
  timeout: 60*60,
  protocol: 'tap'
)

template = executable('test_template', files('test_template.c'),
  dependencies: build_dependencies + test_dependencies,
  include_directories: include_directories,
  c_args: defines + flags
)
test('template', template,
  timeout: 60*60,
  protocol: 'tap'
)

env = environment()
env.set('G_TEST_SRCDIR', meson.current_source_dir())
env.set('G_TEST_BUILDDIR', meson.current_build_dir())
env.set('HOME', 'HOME=/home/test')

envp = find_program('env', required: true)
utils = executable('test_utils', files('test_utils.c'),
  dependencies: build_dependencies + test_dependencies,
  include_directories: include_directories,
  c_args: defines + flags
)
test('utils-with-home', envp,
  args: [
    '-uXDG_CONFIG_HOME',
    '-uXDG_DATA_HOME',
    '-uXDG_CONFIG_DIRS',
    '-uXDG_DATA_DIRS',
    '-uXDG_CACHE_HOME',
    utils
  ],
  timeout: 60*60,
  protocol: 'tap',
  env: env
)
