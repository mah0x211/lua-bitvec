# lua-bitvec

bit-vector module.  

---

## Installation

```sh
luarocks install bitvec --from=http://mah0x211.github.io/rocks/
```

### Usage

```lua
local bitvec = require('bitvec');
local b = bitvec.new( 64 ); -- allocate 64 bit

-- set by boolean value
b[0] = true;
print( b[0] ); -- true
b[0] = false;
print( b[0] ); -- false

-- set by number
b[0] = 1;
print( b[0] ); -- true
b[0] = 0;
print( b[0] ); -- false

-- set by floating-point number
-- floating-point number convert to integral number automatically in internal
b[0] = 1.0;
print( b[0] ); -- true
b[0] = 0.9;
print( b[0] ); -- false

-- auto resize
b[128] = true;
print( b[128] ); -- true
b[128] = false;
print( b[128] ); -- false
```

