--[[

  Copyright (C) 2016 Masatoshi Teruya

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  bitvec.lua
  lua-bitvec
  Created by Masatoshi Teruya on 16/06/18.

--]]
local bitor = require('bit').bor;
local bitand = require('bit').band;
local bitnot = require('bit').bnot;
local bitlshift = require('bit').lshift;
local bitrshift = require('bit').rshift;
-- lookup table of the number of trailing zero
--[[

	Bit Twiddling Hacks By Sean Eron Anderson
	Count the consecutive zero bits on the right with multiply and lookup
	http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup

	MLS: m-sequence(maximal length sequence)
        8 bit: 0x1D
        16 bit: 0x0F2D
        32 bit: 0x077CB531
        64 bit: 0x0218A7A392DD9ABF
                0x02FCA8CF75A6C487
                0x03F566ED27179461
                0x03C953422DFAE33B
                0x03848D96BBCC54FD
                0x03731D7ED10B2A4F

	Create Mapping Table
        32 bit:
            uint8_t tbl[32] = {0};
            uint32_t m = 0x077CB531;
            uint8_t i = 0;

            for(; i < 32; i++ ){
                tbl[m >> 27] = i;
                m <<= 1;
            }

        64 bit:
            uint8_t tbl[64] = {0};
            uint64_t m = 0x0218A7A392DD9ABF;
            uint8_t i = 0;

            for(; i < 64; i++ ){
                tbl[m >> 58] = i;
                m <<= 1;
            }
--]]
local MLS32 = 0x077CB531;

local function createNTZ32LUT()
    local tbl = {};
    local m = MLS32;

    for i = 0, 31 do
        tbl[bitrshift( m, 27 )] = i;
        m = bitlshift( m, 1 );
    end

    return tbl;
end

local NTZ32LUT = createNTZ32LUT();


local function getntz( v )
    -- add 1 to index for the lua language spec reasons
    return NTZ32LUT[bitrshift( bitand( v, -v ) * MLS32, 27 )];
end



-- class BitVec
local BitVec = {}


function BitVec:set( bit )
    local vec = self.vec;
    local idx = bitrshift( bit, 5 );
    local pos = bitlshift( 1, bit % 32 );

    -- vec[bit / 32] |= 1 << ( bit % 32 )
    -- update attribtes
    if idx > self.lastidx then
        self.nbit = ( idx + 1 ) * 32;
        self.lastidx = idx;
        vec[idx] = bitor( 0, pos );
    else
        vec[idx] = bitor( vec[idx], pos );
    end
end


function BitVec:unset( bit )
    local vec = self.vec;
    local idx = bitrshift( bit, 5 );

    if vec[idx] and vec[idx] ~= 0 then
        -- vec[bit / 32] &= ~( 1 << ( bit % 32 ) )
        vec[idx] = bitand( vec[idx], bitnot( bitlshift( 1, bit % 32 ) ) );
    end
end


function BitVec:setrange( from, to )
    local vec = self.vec;
    local fromidx, toidx;

    -- swap value
    if from > to then
        from, to = to, from;
    end
    fromidx = bitrshift( from, 5 );
    toidx = bitrshift( to, 5 );

    if fromidx == toidx then
        -- update attribtes
        if toidx > self.lastidx then
            self.nbit = ( toidx + 1 ) * 32;
            self.lastidx = toidx;
        end
        -- vec[toidx] |= (~( 0xFFFFFFFE << ( to - from ) ) << from);
        vec[toidx] = bitor(
            vec[toidx] or 0,
            bitlshift( bitnot( bitlshift( 0xFFFFFFFE, to - from ) ) , from )
        );
    else
        -- update attribtes
        if toidx > self.lastidx then
            self.nbit = ( toidx + 1 ) * 32;
            self.lastidx = toidx;
        end

        if ( fromidx + 1 ) < toidx then
            for i = fromidx + 1, toidx - 1 do
                vec[i] = -1;
            end
        end

        --
        -- start: from / 32
        --   end: to / 32
        -- vec[start] |= 0xFFFFFFFF << ( from % 32 );
        -- vec[end]   |= ~((0xFFFFFFFE << ( to % 32 ));
        --
        vec[fromidx] = bitor(
            vec[fromidx] or 0, bitlshift( 0xFFFFFFFF, from % 32 )
        );
        vec[toidx] = bitor(
            vec[toidx] or 0, bitnot( bitlshift( 0xFFFFFFFE, to % 32 ) )
        );
    end
end


function BitVec:unsetrange( from, to )
    local vec = self.vec;
    local lastidx = self.lastidx;
    local fromidx;

    -- swap value
    if from > to then
        from, to = to, from;
    end

    fromidx = bitrshift( from, 5 );
    if fromidx <= lastidx then
    	local toidx;

        if to > self.nbit then
            to = self.nbit;
        end
        toidx = bitrshift( to, 5 );

        if fromidx == toidx then
            if vec[toidx] and vec[toidx] ~= 0 then
                -- vec[start] &= ~(~( 0xFFFFFFFE << ( to - from ) ) << from);
                vec[toidx] = bitand(
                    vec[toidx] or 0, bitnot(
                        bitlshift(
                            bitnot( bitlshift( 0xFFFFFFFE, to - from ) ), from
                        )
                    )
                );
            end
        else
            if ( fromidx + 1 ) < toidx then
                for i = fromidx + 1, toidx - 1 do
                    vec[i] = nil;
                end
            end

            if vec[fromidx] and vec[fromidx] ~= 0 then
                -- vec[start] &= ~(0xFFFFFFFF << ( from % 32 ));
                vec[fromidx] = bitand(
                    vec[fromidx], bitnot( bitlshift( 0xFFFFFFFF, from % 32 ) )
                );
            end

            if vec[toidx] and vec[toidx] ~= 0 then
                -- vec[end] &= 0xFFFFFFFE << ( to % 32 );
                vec[toidx] = bitand(
                    vec[toidx], bitlshift( 0xFFFFFFFE, to % 32 )
                );
            end
        end
    end
end


function BitVec:get( bit )
    local vec = self.vec;
    local idx = bitrshift( bit, 5 );

    if vec[idx] then
        -- ( vec[bit / 32] >> ( bit % 32 ) ) & 1
        return bitand( bitrshift( vec[idx], bit % 32 ), 1 ) == 1;
    end

    return false;
end


--- ntz number of trailing zero
function BitVec:ntz()
    local vec = self.vec;

    for i = 0, self.lastidx do
        if vec[i] and vec[i] ~= 0 then
            return ( i * 32 ) + getntz( vec[i] );
        end
    end

    return self.nbit;
end


--- ffz find first zero
function BitVec:ffz()
    local vec = self.vec;
    local nbit = self.nbit;

    for i = 0, self.lastidx do
        if vec[i] and vec[i] ~= -1 then
            i = ( i * 32 ) + getntz( bitnot( vec[i] ) );
            if i > nbit then
                break;
            end

            return i;
        end
    end

    return nbit == 0xFFFFFFFF and -1 or nbit + 1;
end


function BitVec:clear()
    self.nbit = 32;
    self.lastidx = 0;
    self.vec = { [0] = 0 };
end


local function new()
    local bv = setmetatable({}, {
        __index = BitVec
    });

    bv:clear();
    return bv;
end


return {
    new = new
};
