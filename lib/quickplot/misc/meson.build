
quickplotHelp = executable('quickplotHelp',
  ['quickplotHelp.c'],
  install: true,
  # FIXME: Is this the best installation directory, or is there a more
  # meson-like (portable) installation directory?
  install_dir: join_paths(get_option('libdir'), 'quickplot', 'misc'),
  c_args: c_args,
  link_with: [built_libs['debug'], built_libs['cmdloptHelp']],
  # this is in: libdir/quickplot/misc/   libdebug.so is in libdir/
  install_rpath: join_paths('$ORIGIN', '..', '..') )

