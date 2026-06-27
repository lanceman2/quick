
custom_target('quickplot_bashComp',
  command: ['mk_quickplot.bash', '@INPUT@', rel_libdir],
  install: true,
  # FIXME: Is this the best installation directory, or is there a more
  # meson-like (portable) installation directory?
  install_dir: join_paths(get_option('prefix'), 'share', 'bash-completion', 'completions'),
  output: 'quickplot',
  input: 'quickplot.IN',
  # mk_quickplot.bash runs quickplotHelp
  depends: [ quickplotHelp ])

