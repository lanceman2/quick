
project('module', 'c', meson_version: '>=1.7.0', version: '0.0.1',
  default_options : [ 'prefix=/usr/local/encap/quick' ])



# The code needs to know there the libdir is relative to the prefix.
# We use relative path so as to brake more uncommon install cases.
#
prefix = get_option('prefix')  # example: '/usr'
rel_libdir = get_option('libdir')  # example: '/usr/lib/x86_64-linux-gnu'  or '/usr/lib'

prefix = prefix.endswith('/') ? prefix : prefix + '/'
rel_libdir = rel_libdir.replace(prefix, '')
# if rel_libdir can be exactly prefix
rel_libdir = (rel_libdir == '') ? '.' : rel_libdir
# example: rel_libdir = 'lib/x86_64-linux-gnu' or 'lib'
# Don't pollute the namespace.
prefix=''


subdir('include')
subdir('lib')
subdir('bin')
subdir(join_paths('share', 'bash-completion', 'completions'))

if get_option('tests')
  subdir('tests')
endif

