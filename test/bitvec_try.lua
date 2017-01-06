local bitvec = require('bitvec');
local b = ifNil( bitvec.new() );

-- set bit
b:set( 0 );
ifNotTrue( b:get(0) );
b:unset( 0 );
ifNotFalse( b:get(0) );

b:clear();

-- autoresize
b:set( 128 );
ifNotTrue( b:get( 128 ) );

b:unset( 128 );
ifNotFalse( b:get( 128 ) );

b:clear();


-- bit range
b:setrange( 2, 10 )
for i = 2, 10 do
    ifNotTrue( b:get( i ) );
end
b:unsetrange( 2, 10 )
for i = 2, 10 do
    ifNotFalse( b:get( i ) );
end

b:setrange( 61, 121 )
for i = 61, 121 do
    ifNotTrue( b:get( i ) )
end
b:unsetrange( 61, 121 )
for i = 61, 121 do
    ifNotFalse( b:get( i ) )
end

b:clear();


-- number of trailing zeros
ifNotEqual( b:ntz(), 64 );
b:set( 97 );
ifNotEqual( b:ntz(), 97 );

b:unset( 97 );
ifNotEqual( b:ntz(), 128 );

b:clear();


-- find first zero
ifNotEqual( b:ffz(), 0 );
b:set( 0 );
ifNotEqual( b:ffz(), 1 );
b:setrange( 2, 10 );
ifNotEqual( b:ffz(), 1 )
b:set( 1 );
ifNotEqual( b:ffz(), 11 );

b:clear();


b:set( 0 );
b:setrange( 2, 300 );
ifNotEqual( b:ffz(), 1 );

b:set( 1 );
ifNotEqual( b:ffz(), 301 );

b:unsetrange( 128, 256 );
ifNotEqual( b:ffz(), 128 );

b:unsetrange( 191, 191 );
ifNotEqual( b:ffz(), 128 );

b:set( 128 );
ifNotEqual( b:ffz(), 129 );

b:setrange( 128, 198 );
ifNotEqual( b:ffz(), 199 );

b:setrange( 190, 301 );
ifNotEqual( b:ffz(), 302 );

b:unset( 78 );
ifNotEqual( b:ffz(), 78 );


