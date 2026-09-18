
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

# We found that meson wrap-file did not do what we needed.  We wish to get
# a few files that are pretty stable, that we do not maintain, and we
# don't want the full repositories as a git sub-modules to this project.
downloaded = custom_target('downloaded',
    # The header files are installed from the install_header() call above.
    install: false,
    command: ['download_files'],
    # 'downloaded' is a directory created by the 'download' script.
    output: [ 'downloaded'],
    # This seems to be required:
    build_by_default: true)

subdir('include')
subdir('lib')
subdir('bin')
subdir(join_paths('share', 'bash-completion', 'completions'))

if get_option('tests')
  subdir('tests')
endif

