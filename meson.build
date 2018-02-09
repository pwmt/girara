project('girara', 'c',
  meson_version: '>=0.43',
  default_options: 'c_std=c11'
)

version_major = '0'
version_minor = '2'
version_rev = '8'
version = '@0@.@1@.@2@'.format(version_major, version_minor, version_rev)

# Rules for so_major and so_minor:
# Before a release perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
#   bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.
so_major = '3'
so_minor = '0'
so_version = '@0@.@1@'.format(so_major, so_minor)

conf_data = configuration_data()
conf_data.set('GVMAJOR', version_major)
conf_data.set('GVMINOR', version_minor)
conf_data.set('GVREV', version_rev)
conf_data.set('version', version)

cc = meson.get_compiler('c', required: false)

prefix = get_option('prefix')
localedir = get_option('localedir')

libm = cc.find_library('libm')
glib = dependency('glib-2.0', version: '>=2.50')
gtk3 = dependency('gtk+-3.0', version: '>=3.20')

build_dependencies = [libm, glib, gtk3]
pc_requires = ['glib-2.0', 'gtk+-3.0']

defines = [
  '-DGETTEXT_PACKAGE="libgirara-gtk3-@0@"'.format(so_major),
  '-DLOCALEDIR="@0@"'.format(join_paths(prefix, localedir)),
  '-D_DEFAULT_SOURCE',
]

flags = [
  '-Wall',
  '-Wextra',
  '-pedantic',
  '-Wmissing-declarations',
  '-Werror=implicit-function-declaration'
]
flags = cc.get_supported_arguments(flags)

if get_option('enable-notify')
  libnotify = dependency('libnotify', version: '>=0.7.0', required: false)
  if libnotify.found()
    build_dependencies += libnotify
    defines += '-DWITH_LIBNOTIFY'
    pc_requires += 'libnotify'
  endif
endif

if get_option('enable-json')
  jsonc = dependency('json-c', required: false)
  if jsonc.found()
    build_dependencies += jsonc
    defines += '-DWITH_JSONC'
    pc_requires += 'json-c'
  endif
endif

version_header = configure_file(
  input: 'girara/version.h.in',
  output: 'version.h',
  configuration: conf_data
)
include_directories = [
  include_directories('.')
]

subdir('data')
subdir('po')

sources = files(
  'girara/callbacks.c',
  'girara/completion.c',
  'girara/entry.c',
  'girara/input-history-io.c',
  'girara/session.c',
  'girara/shortcuts.c',
  'girara/template.c',
  'girara/commands.c',
  'girara/config.c',
  'girara/datastructures.c',
  'girara/input-history.c',
  'girara/log.c',
  'girara/settings.c',
  'girara/statusbar.c',
  'girara/utils.c'
)
sources += girara_css

headers = files(
  'girara/callbacks.h',
  'girara/commands.h',
  'girara/completion.h',
  'girara/config.h',
  'girara/css-definitions.h',
  'girara/datastructures.h',
  'girara/entry.h',
  'girara/girara.h',
  'girara/input-history.h',
  'girara/log.h',
  'girara/macros.h',
  'girara/session.h',
  'girara/settings.h',
  'girara/shortcuts.h',
  'girara/statusbar.h',
  'girara/template.h',
  'girara/types.h',
  'girara/utils.h'
)
headers += version_header

girara = library(
  'girara-gtk3',
  sources,
  dependencies: build_dependencies,
  soversion: so_version,
  install: true,
  include_directories: include_directories,
  c_args: defines + flags
)
install_headers(headers, subdir: 'girara')

pkg = import('pkgconfig')
pkg.generate(
  name: 'girara-gtk3',
  description: 'User interface library',
  url: 'https://pwmt.org/projects/girara',
  version: version,
  libraries: girara,
  requires_private: pc_requires,
)

subdir('tests')
