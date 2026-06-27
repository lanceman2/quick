
TunerHelp = executable('TunerHelp',
  ['TunerHelp.c'],
  install: true,
  # FIXME: Is this the best installation directory, or is there a more
  # meson-like (portable) installation directory?
  install_dir: join_paths(get_option('libdir'), 'Tuner', 'misc'),
  c_args: c_args,
  link_with: [built_libs['debug'], built_libs['cmdloptHelp']],
  install_rpath: join_paths('$ORIGIN', '..', '..') )

