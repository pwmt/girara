check = dependency('check', required: false)
if check.found()
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
  test('config', config,
    timeout: 60*60,
    is_parallel: false
  )

  datastructures = executable('test_datastructures', ['test_datastructures.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('datastructures', datastructures,
    timeout: 60*60
  )

  session = executable('test_session', ['test_session.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('session', session,
    timeout: 60*60,
    is_parallel: false
  )

  setting = executable('test_setting', ['test_setting.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('setting', setting,
    timeout: 60*60,
    is_parallel: false
  )

  template = executable('test_template', ['test_template.c', 'tests.c'],
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('template', template,
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
  test('utils-with-home', utils,
    env: env,
    timeout: 60*60
  )

  env_bin = find_program('env', required: false)
  if env_bin.found()
    env = environment()
    env.set('XDG_TEST_HELPER_PATH', meson.current_build_dir())

    test('utils-without-home', env_bin,
      args: ['-u', 'HOME', utils],
      env: env,
      timeout: 60*60
    )
  endif
endif
