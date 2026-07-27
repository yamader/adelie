add_rules('mode.debug', 'mode.release')

add_repositories 'myrepo repo'
add_requires('boost_beast2', 'boost_di')

set_languages 'c++20'

target 'adelie'
set_kind 'shared'
add_files('src/**.ccm', { public = true })
add_files 'src/**.cc'
add_packages('boost_beast2', 'boost_di')

target 'example'
set_kind 'binary'
add_deps 'adelie'
add_files('example/**.cc', 'example/**.ccm')
