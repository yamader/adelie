package 'boost_http'
add_urls 'https://github.com/cppalliance/http.git'
add_versions('2026.06.13', '571adcc96ad1f2df935ebbfdf9cb76a20b78608d')
add_deps('cmake', 'boost_capy')
add_deps('boost', { configs = { json = true, url = true } })

add_patches('*', 'patches/cmake-install.patch', 'e45c02f05099fd087ca2492036323cd9abac8ee2795a0f3261351a1b4965a144')

on_install(function(package)
  import('package.tools.cmake').install(package, {
    '-DBOOST_HTTP_BUILD_TESTS=OFF',
    '-DBOOST_HTTP_BUILD_EXAMPLES=OFF',
  })
end)
