
name = 'save_png_to_file'

e = executable(name, name + '.c',
  c_args: '-g',
  link_with: [built_libs['quickdraw']], dependencies: [libpng_dep])
test(name, e, suite: [ 'general', 'non_interactive', 'all' ])

name = '010_dlopen'
e = executable(name, name + '.c',
  c_args: '-g',
  link_with: [built_libs['debug']])
test(name, e, suite: [ 'general', 'interactive', 'all' ])

name = '001_valgrind_malloc_fail'
e = executable(name, name + '.c',
  c_args: '-g',
  link_with: [built_libs['debug']])
test(name, e, suite: [ 'general', 'non_interactive', 'all' ], should_fail: true)

# Test this installed program:
test('WL_list_registry', WL_list_registry, suite: [ 'general', 'non_interactive', 'all' ])


# apt install libegl1-mesa-dev
name = 'wayland_egl'
e = executable(name, [name + '.c', wl_sources, wl_headers],
  c_args: '-g',
  link_with: [built_libs['debug']],
  dependencies: [ wl_dep, libwayland_cursor_dep, dependency('wayland-egl'), dependency('egl'), dependency('gl')])
test(name, e, suite: [ 'general', 'interactive', 'all' ])

