add_rules('mode.debug', 'mode.release')

add_repositories 'myrepo repo'
add_requires('boost_beast2', 'boost_capy', 'boost_di', 'boost_http')
add_requires('boost', { configs = { json = true, url = true } }) -- same config as boost_{beast2,http}

set_languages 'c++20'

target 'adelie'
set_kind 'shared'
add_files('src/**.ccm', { public = true })
add_files 'src/**.cc'
add_packages('boost_http', 'boost_capy', 'boost', 'boost_di', { public = true })
add_packages 'boost_beast2'

target 'example'
set_kind 'binary'
add_deps 'adelie'
add_files('example/**.cc', 'example/**.ccm')
