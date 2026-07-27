package 'boost_capy'
add_urls 'https://github.com/cppalliance/capy.git'
add_versions('2026.06.21', '9144290189fa149b27617c7d9a476c8fbffb7b8c')
add_deps 'cmake'

add_patches('*', 'patches/cmake-boost-version.patch', 'e857dd9fdfe3cefc0d9ba8093cb911cba2f35ff149ee1da712c566cd30277b32')
add_patches('*', 'patches/cmake-find-threads.patch', '089bf1f5ddf4f5054a25eece1541694bea3da3e8736f535a384374dd2ecc1e7a')

on_install(function(package)
  import('package.tools.cmake').install(package, {
    '-DBOOST_CAPY_BUILD_TESTS=OFF',
    '-DBOOST_CAPY_BUILD_EXAMPLES=OFF',
    '-DBOOST_CAPY_BUILD_BENCH=OFF',
  })
end)
