project('vivid', 'cpp',
  version : '1.0.0',
  default_options : ['warning_level=2', 'cpp_std=c++17'])

# Dependencies
gtk4_dep = dependency('gtk4')
x11_dep = dependency('x11', required: false)
xrandr_dep = dependency('xrandr', required: false)
threads_dep = dependency('threads')

# Source files - Updated to match new structure
sources = [
  'src/main.cpp',
  'src/core/VibranceController.cpp',
  'src/ui/MainWindow.cpp'
]

# Include directories
inc = include_directories('src')

# Dependencies list
deps = [gtk4_dep, threads_dep]

if x11_dep.found() and xrandr_dep.found()
  deps += [x11_dep, xrandr_dep]
  add_project_arguments('-DHAVE_X11', language: 'cpp')
  message('X11 support: enabled')
else
  message('X11 support: disabled')
endif

# Main executable
executable('vivid',
  sources,
  dependencies: deps,
  include_directories: inc,
  install: true)

message('Build configured successfully!')
message('Run: meson compile -C builddir')
message('Then: ./builddir/vivid')
