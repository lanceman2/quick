
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

