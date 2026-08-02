# Adelie

Boost.Httpのせいでlibc++非対応

システムのBoostがLLVMでビルドされている場合，`boost_beast2`と`boost_http`の`add_deps('boost')`に`system = false`を設定すればよい
