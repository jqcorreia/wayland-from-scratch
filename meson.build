project('wayland', 'c')
files = [
  'main.c',
  'shm.c',
  'xdg-shell.c'
]

deps = [
  dependency('wayland-client'),
]

executable('main', files, dependencies : deps, link_args: ['-lschrift', '-lm'])

