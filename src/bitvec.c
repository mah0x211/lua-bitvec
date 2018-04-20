/*
 *  Copyright 2015 Masatoshi Teruya. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 *  bitvec.c
 *  lua-bitvec
 *
 *  Created by Masatoshi Teruya on 15/02/25.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <lua.h>
#include <lauxlib.h>
#include "../deps/bitvec/bitvec.h"

#define MODULE_MT   "bitvec"

// helper macros
#define lstate_setmetatable(L,tname) do{ \
    luaL_getmetatable( L, tname ); \
    lua_setmetatable( L, -2 ); \
}while(0)

#define lstate_fn2tbl(L,k,v) do{ \
    lua_pushstring(L,k); \
    lua_pushcfunction(L,v); \
    lua_rawset(L,-3); \
}while(0)



static int ntz_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );

    lua_pushinteger( L, bitvec_ntz( bv ) );

    return 1;
}


static int ffz_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    size_t pos = bitvec_ffz( bv );

    if( pos == SIZE_MAX && bv->nbit < BV_TYPEMAX &&
        bitvec_resize( bv, bv->nbit + BV_BIT ) == 0 ){
        pos = bitvec_ffz( bv );
    }

    lua_pushinteger( L, pos );

    return 1;
}


static int unsetrange_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer from = luaL_checkinteger( L, 2 );
    lua_Integer to = luaL_checkinteger( L, 3 );

    luaL_argcheck( L, from >= 0, 2, "unsigned int expected, got signed int" );
    luaL_argcheck( L, to >= 0, 3, "unsigned int expected, got signed int" );
    // swap value
    if( from > to ){
        from = to;
        to = from;
    }

    if( to > bv->nbit ){
        to = bv->nbit;
    }

    if( from <= bv->nbit ){
        bitvec_unset_range( bv, from, to );
    }

    return 0;
}


static int setrange_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer from = luaL_checkinteger( L, 2 );
    lua_Integer to = luaL_checkinteger( L, 3 );

    luaL_argcheck( L, from >= 0, 2, "unsigned int expected, got signed int" );
    luaL_argcheck( L, to >= 0, 3, "unsigned int expected, got signed int" );
    // swap value
    if( from > to ){
        from = to;
        to = from;
    }

    // auto-resize
    if( to <= bv->nbit || bitvec_resize( bv, to ) == 0 ){
        bitvec_set_range( bv, from, to );
        lua_pushboolean( L, 1 );
        return 1;
    }

    // no-mem error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int unset_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer pos = luaL_checkinteger( L, 2 );

    luaL_argcheck( L, pos >= 0, 2, "unsigned int expected, got signed int" );
    if( pos <= bv->nbit ){
        bitvec_unset( bv, pos );
    }

    return 0;
}


static int set_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer pos = luaL_checkinteger( L, 2 );

    luaL_argcheck( L, pos >= 0, 2, "unsigned int expected, got signed int" );
    // auto-resize
    if( pos <= bv->nbit || bitvec_resize( bv, bv->nbit + BV_BIT ) == 0 ){
        bitvec_set( bv, pos );
        lua_pushboolean( L, 1 );
        return 1;
    }

    // no-mem error
    lua_pushboolean( L, 0 );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int get_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer pos = luaL_checkinteger( L, 2 );

    luaL_argcheck( L, pos >= 0, 2, "unsigned int expected, got signed int" );
    lua_pushboolean( L, bitvec_get( bv, pos ) == 1 );

    return 1;
}


static int clear_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );

    if( bitvec_resize( bv, BV_BIT ) == 0 ){
        bitvec_unset_range( bv, 0, bv->nbit );
    }

    return 0;
}


static int nbit_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );

    lua_pushinteger( L, bv->nbit );

    return 1;
}


static int gc_lua( lua_State *L )
{
    bitvec_dispose( (bitvec_t*)lua_touserdata( L, 1 ) );
    return 0;
}


static int tostring_lua( lua_State *L )
{
    lua_pushfstring( L, MODULE_MT ": %p", lua_touserdata( L, 1 ) );
    return 1;
}


static int new_lua( lua_State *L )
{
    bitvec_t *bv = lua_newuserdata( L, sizeof( bitvec_t ) );

    // alloc new context
    if( bitvec_init( bv, 0 ) == 0 ){
        lstate_setmetatable( L, MODULE_MT );
        return 1;
    }

    // nomem error
    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


LUALIB_API int luaopen_bitvec( lua_State *L )
{
    struct luaL_Reg mmethods[] = {
        { "__gc", gc_lua },
        { "__tostring", tostring_lua },
        { NULL, NULL }
    };
    struct luaL_Reg methods[] = {
        { "nbit", nbit_lua },
        { "clear", clear_lua },
        { "get", get_lua },
        { "set", set_lua },
        { "unset", unset_lua },
        { "setrange", setrange_lua },
        { "unsetrange", unsetrange_lua },
        { "ffz", ffz_lua },
        { "ntz", ntz_lua },
        { NULL, NULL }
    };
    struct luaL_Reg *ptr = mmethods;

    luaL_newmetatable( L, MODULE_MT );
    // metamethods
    while( ptr->name ){
        lstate_fn2tbl( L, ptr->name, ptr->func );
        ptr++;
    }
    // methods
    ptr = methods;
    lua_pushstring( L, "__index" );
    lua_newtable( L );
    // metamethods
    while( ptr->name ){
        lstate_fn2tbl( L, ptr->name, ptr->func );
        ptr++;
    }
    lua_rawset( L,-3 );
    lua_pop( L, 1 );

    // module function
    lua_createtable( L, 0, 1 );
    lstate_fn2tbl( L, "new", new_lua );

    return 1;
}




