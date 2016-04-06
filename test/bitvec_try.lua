local bitvec = require('bitvec');
-- default 64 bit
local b = ifNil( bitvec.new() );
ifNotEqual( b.nbit, 64 );

-- set by boolean value
b[0] = true;
ifNotTrue( b[0] );
b[0] = false;
ifNotFalse( b[0] );

-- set by number
b[0] = 1;
ifNotTrue( b[0] );
b[0] = 0;
ifNotFalse( b[0] );


-- set by floating-point number
-- floating-point number convert to integral number automatically in internal
b[0] = 1.0;
ifNotTrue( b[0] );
b[0] = 0.9;
ifNotFalse( b[0] );


-- autoresize
b[128] = true;
ifNotTrue( b[128] );
b[128] = false;
ifNotFalse( b[128] );
ifNotEqual( b.nbit, 128 );


-- number of trailing zeros
ifNotEqual( b.ntz, 128 );
b[96] = true;
ifNotEqual( b.ntz, 96 );
b[96] = false;
ifNotEqual( b.ntz, 128 );

-- find first zero
ifNotEqual( b.ffz, 0 );
for i = 0, 10 do
    b[i] = true;
    ifNotEqual( b.ffz, i + 1 );
end
for i = 0, 10 do
    b[i] = false;
end


local function invalidFn()end
local invalidCo = coroutine.create( invalidFn );
local invalidTbl = {};


-- set by invalid value
b[0] = invalidFn;
ifTrue( b[0] );
b[0] = invalidCo;
ifTrue( b[0] );
b[0] = 'string';
ifTrue( b[0] );
b[0] = invalidTbl;
ifTrue( b[0] );
b[0] = b;
ifTrue( b[0] );


-- set at invalid index
b[1.9] = true;
ifTrue( b[-1] );
b[-1] = true;
ifTrue( b[-1] );
b[-2.0] = true;
ifTrue( b[-2.0] );
b[invalidFn] = true;
ifTrue( b[invalidFn] );
b[invalidCo] = true;
ifTrue( b[invalidCo] );
b[invalidTbl] = true;
ifTrue( b[invalidTbl] );
b[b] = true;
ifTrue( b[b] );


