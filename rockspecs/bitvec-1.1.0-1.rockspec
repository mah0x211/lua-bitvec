package = "bitvec"
version = "1.1.0-1"
source = {
    url = "gitrec://github.com/mah0x211/lua-bitvec.git",
    tag = 'v1.1.0'
}
description = {
    summary = "bit-vector module",
    homepage = "https://github.com/mah0x211/lua-bitvec", 
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1",
    "luarocks-fetch-gitrec >= 0.2"
}
build = {
    type = "builtin",
    modules = {
        bitvec = {
            sources = { 
                "src/bitvec.c",
            }
        }
    }
}

