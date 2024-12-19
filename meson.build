project('wayland', 'c')
files = [
  'main.c',
  'init.c',
  'shm.c',
  'utils.c',
  'xdg.c',
  'gen/wlr-layer-shell-unstable-v1.c',
  'gen/xdg-shell.c',
]

deps = [
  dependency('wayland-client'),
]

executable('main', files, dependencies : deps, link_args: ['-lschrift', '-lm', '-O3'])

