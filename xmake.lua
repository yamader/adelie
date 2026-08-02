add_rules('mode.debug', 'mode.release')

add_repositories('myrepo repo')
add_requires('boost', { configs = { json = true, url = true } }) -- same config as boost_{beast2,http}
add_requires('boost_beast2', 'boost_capy', 'boost_di', 'boost_http')
add_requires('sqlite3', { optional = true })

set_languages('c++23')

-- GCC < 16.2 lazy module loading hits bug #124953 ("failed to load pendings")
add_cxxflags('-fno-module-lazy')

target('adelie')
    set_kind('shared')
    add_files('src/**.cc')
    add_files('src/**.ccm', { public = true })
    add_packages('boost', 'boost_beast2', 'boost_capy')
    add_packages('boost_di', 'boost_http', { public = true })
    add_packages('sqlite3', { optional = true })

target('example')
    set_kind('binary')
    add_deps('adelie')
    add_files('example/**.cc', 'example/**.ccm')
    set_rundir('$(projectdir)')
