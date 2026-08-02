package('boost_corosio')
    add_urls('https://github.com/cppalliance/corosio.git')
    add_versions('2026.06.21', '4276bd4039097fd4dd62dda4bb55e53d03351088')

    add_patches('*', 'patches/cmake-boost-version.patch', 'fd1bb9dade9a953f0bba3899591adb0863e8726b3f52322a3fe3e0d108915cc0')
    add_patches('*', 'patches/cmake-find-capy.patch', '9022e318e71e2a912513170361bb81bc460fbb03a4d0d7249a38b0ba113bf8ea')

    add_deps('cmake', 'boost_capy')
    add_deps('liburing', { optional = true })

    on_install(function(package)
        import('package.tools.cmake').install(package, {
            '-DBOOST_COROSIO_BUILD_TESTS=OFF',
            '-DBOOST_COROSIO_BUILD_PERF=OFF',
            '-DBOOST_COROSIO_BUILD_EXAMPLES=OFF',
        })
    end)
