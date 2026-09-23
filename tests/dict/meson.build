
name = 'dict_01'
e = executable(name, name + '.c',
  c_args: '-g',
  link_with: [built_libs['debug'], built_libs['dict']])
test(name, e, suite: [ 'general', 'non-interactive', 'all' ])

