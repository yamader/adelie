package('boost_beast2')
    add_urls('https://github.com/cppalliance/beast2.git')
    add_versions('2026.05.15', 'fb0a9c8dc860590bd13cb2d7afd3e2232291ff19')

    add_patches('*', 'patches/cmake-install.patch', 'c92f6bbc2641f9688b7025959f1c5b26311d74083c0647ccb631a4984579e00c')

    add_deps('cmake', 'boost_capy', 'boost_corosio', 'boost_http')
    add_deps('boost', { configs = { json = true, url = true } })

    on_install(function(package)
        import('package.tools.cmake').install(package, {
            '-DBOOST_BEAST2_BUILD_TESTS=OFF',
            '-DBOOST_BEAST2_BUILD_EXAMPLES=OFF',
        })
    end)
