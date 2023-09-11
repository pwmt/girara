check = dependency('check', required: get_option('tests'), version: '>=0.11')
xvfb = find_program('xvfb-run', required: get_option('tests'))
if check.found() and xvfb.found()
  test_dependencies = [
    declare_dependency(link_with: girara),
    check
  ]

  include_directories += [ include_directories('../girara') ]

  xdg_test_helper = executable('xdg_test_helper', 'xdg_test_helper.c',
    dependencies: [ glib, declare_dependency(link_with: girara) ],
    include_directories: include_directories,
    c_args: defines + flags
  )

  config = executable('test_config', ['test_config.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('config', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', config],
    timeout: 60*60,
    is_parallel: false
  )

  datastructures = executable('test_datastructures', ['test_datastructures.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('datastructures', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', datastructures],
    timeout: 60*60
  )

  session = executable('test_session', ['test_session.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('session', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', session],
    timeout: 60*60,
    is_parallel: false
  )

  setting = executable('test_setting', ['test_setting.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('setting', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', setting],
    timeout: 60*60,
    is_parallel: false
  )

  template = executable('test_template', ['test_template.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('template', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', template],
    timeout: 60*60
  )

  utils = executable('test_utils', ['test_utils.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )

  env = environment()
  env.set('XDG_TEST_HELPER_PATH', meson.current_build_dir())
  env.set('HOME', '/home/test')
  test('utils-with-home', xvfb,
    args: ['-a', '-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset', utils],
    env: env,
    timeout: 60*60
  )
endif
