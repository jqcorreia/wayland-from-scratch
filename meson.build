project('wayland', 'c')
files = [
  'main.c',
  'shm.c',
  'xdg-shell.c',
  'utils.c',
  'wlr-layer-shell-unstable-v1.c'
]

deps = [
  dependency('wayland-client'),
]

executable('main', files, dependencies : deps, link_args: ['-lschrift', '-lm', '-O3'])

