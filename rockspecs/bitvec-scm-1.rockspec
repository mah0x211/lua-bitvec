package = "bitvec"
version = "scm-1"
source = {
    url = "git://github.com/mah0x211/lua-bitvec.git"
}
description = {
    summary = "bitvec module",
    homepage = "https://github.com/mah0x211/lua-bitvec",
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
    "luabitop >= 1.0.2"
}
build = {
    type = "builtin",
    modules = {
        bitvec = "bitvec.lua";
    }
}

