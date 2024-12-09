if get_option('tests').allowed()
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
    protocol: 'tap',
    env: [
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir())
    ]
  )

  template = executable('test_template', files('test_template.c'),
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('template', template,
    timeout: 60*60,
    protocol: 'tap',
    env: [
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir())
    ]
  )

  utils = executable('test_utils', files('test_utils.c'),
    dependencies: build_dependencies + test_dependencies,
    include_directories: include_directories,
    c_args: defines + flags
  )
  test('utils-with-home', utils,
    timeout: 60*60,
    protocol: 'tap',
    env: [
      'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
      'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
      'HOME=/home/test'
    ]
  )

  xvfb = find_program('xvfb-run', required: get_option('tests'))
  if xvfb.found()
    xvfb_args = ['-s', '-screen 0 1400x900x24 -ac +extension GLX +render -noreset']
    xvfb_h_output = run_command(xvfb, '-h', capture: true, check: false)
    if xvfb_h_output.stdout().contains('--auto-display')
      # becasue Arch and Fedora
      xvfb_args += ['-d']
    else
      xvfb_args += ['-a']
    endif

    config = executable('test_config', files('test_config.c'),
      dependencies: build_dependencies + test_dependencies,
      include_directories: include_directories,
      c_args: defines + flags
    )
    test('config', xvfb,
      args: xvfb_args + [config],
      timeout: 60*60,
      is_parallel: false,
      protocol: 'tap',
      env: [
        'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
        'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
        'NO_AT_BRIDGE=1',
        'MESA_LOG=null',
        'LIBGL_DEBUG=quiet'
      ]
    )

    session = executable('test_session', files('test_session.c'),
      dependencies: build_dependencies + test_dependencies,
      include_directories: include_directories,
      c_args: defines + flags
    )
    test('session', xvfb,
      args: xvfb_args + [session],
      timeout: 60*60,
      is_parallel: false,
      protocol: 'tap',
      env: [
        'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
        'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
        'NO_AT_BRIDGE=1',
        'MESA_LOG=null',
        'LIBGL_DEBUG=quiet'
      ]
    )

    setting = executable('test_setting', files('test_setting.c'),
      dependencies: build_dependencies + test_dependencies,
      include_directories: include_directories,
      c_args: defines + flags
    )
    test('setting', xvfb,
      args: xvfb_args + [setting],
      timeout: 60*60,
      is_parallel: false,
      protocol: 'tap',
      env: [
        'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
        'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
        'NO_AT_BRIDGE=1',
        'MESA_LOG=null',
        'LIBGL_DEBUG=quiet'
      ]
    )
  endif
endif
