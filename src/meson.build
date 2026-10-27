process_manager_sources = [
  'main.c',
  'process-manager-application.c',
  'process-manager-window.c',
  'view/process-view.c',
  'core/process-item.c',
  'core/process-model.c',
  'core/process-raw.c',
  'core/system-status.c',
  'components/table-cell.c',
  'util/disk-usage-type.c',
  'util/cpu-usage-type.c',
  'components/table-cell.c',
  'components/process-details.c',
  'components/expander-cell.c',
  'components/search-bar.c',
  'components/numeric-cell.c'
]

process_manager_deps = [
  dependency('gtk4', version: '>= 4.10'),
]

process_manager_sources += gnome.compile_resources('process-manager-resources',
  'process-manager.gresource.xml',
  c_name: 'process_manager'
)

executable('process-manager', process_manager_sources,
  dependencies: process_manager_deps,
       install: true,
)
