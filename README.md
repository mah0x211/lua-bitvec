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
local b = bitvec.new();

-- set
b:set( 0 );
print( b:get( 0 ) ); -- true


-- unset
b:unset( 0 );
print( b:get( 0 ) );


-- number of trailing zeros
print( b:ntz() ) -- 32


-- auto resize
b:set( 97 );
b:unset( 97 );
print( b:ntz() ) -- 128


-- initialized to default
b:clear();
print( b:ntz() ) -- 32


-- set bit range
b:setrange( 61, 121 )
for i = 61, 121 do
    print( b:get( i ) ) -- true
end


-- unset bit range
b:unsetrange( 61, 121 )
for i = 61, 121 do
    print( b:get( i ) ) -- false
end
b:clear();


-- find first zero
print( b:ffz() ) -- 0
b:set( 0 );
b:setrange( 2, 10 );
print( b:ffz() ) -- 1
b:set( 1 );
print( b:ffz() ) -- 11
```

