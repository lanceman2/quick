
add_test_setup('non_interactive', exclude_suites: ['interactive'], is_default: true)
add_test_setup('interactive', exclude_suites: ['non_interactive'])
add_test_setup('valgrind',
  exe_wrapper: ['valgrind',
    '--leak-check=full',
    '--show-leak-kinds=all',
    '--show-reachable=yes',
    '--errors-for-leak-kinds=all',
    '--error-exitcode=1',
    '--suppressions=' +
    join_paths(meson.project_source_root(), 'valgrind_suppressions'),
    '-s'],
  env: ['VALGRIND=1'])
  # Got issues with running valgrind with '--show-leak-kinds=all'; problem
  # with /usr/lib/x86_64-linux-gnu/libdl.so.2


subdir('quickdraw')
subdir('quickdraw_interactive')
subdir('text')
subdir('panels')
subdir('general')
# FIXME: This is being replaced by libtext
subdir('fontRendering')

