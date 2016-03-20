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


#define LUANUM_ISDBL(val)   ((lua_Number)((lua_Integer)val) != val)

#define LUANUM_ISUINT(val)  (!signbit( val ) && !LUANUM_ISDBL( val ))


static int newindex_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    
    if( lua_type( L, 2 ) == LUA_TNUMBER )
    {
        lua_Number idx = lua_tonumber( L, 2 );
        
        if( LUANUM_ISUINT( idx ) )
        {
            BV_TYPE pos = (BV_TYPE)idx;
            int type = lua_type( L, 3 );
            int flag = 0;
            
            if( type == LUA_TBOOLEAN ){
                flag = lua_toboolean( L, 3 );
            }
            else if( type == LUA_TNUMBER ){
                flag = lua_tointeger( L, 3 );
            }
            // invalid argument
            else {
                return 0;
            }
            
            // auto-resize
            if( pos > bv->nbit && bitvec_resize( bv, pos ) == -1 ){
                // no-mem error
                return luaL_error( L, strerror( errno ) );
            }
            else if( flag ){
                bitvec_set( bv, pos );
            }
            else {
                bitvec_unset( bv, pos );
            }
        }
    }
    
    return 0;
}


static int index_lua( lua_State *L )
{
    bitvec_t *bv = luaL_checkudata( L, 1, MODULE_MT );
    lua_Integer pos = 0;
    size_t len = 0;
    const char *str = NULL;

    switch( lua_type( L, 2 ) )
    {
        case LUA_TNUMBER:
            pos = lua_tointeger( L, 2 );
            lua_pushboolean( L, bitvec_get( bv, pos ) == 1 );
        break;

        case LUA_TSTRING:
            str = lua_tolstring( L, 2, &len );
            // ntz
            if( len == 3 && str[0] == 'n' && str[1] == 't' && str[2] == 'z' ){
                lua_pushinteger( L, bitvec_ntz( bv ) );
                break;
            }

        // out-of-range
        default:
            lua_pushboolean( L, 0 );
    }
    
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
    lua_Integer nbits = luaL_optinteger( L, 1, 64 );
    bitvec_t *bv = lua_newuserdata( L, sizeof( bitvec_t ) );

    // invalid argument
    if( nbits < 1 ){
        errno = EINVAL;
    }
    // alloc new context
    else if( bv && bitvec_init( bv, nbits ) == 0 ){
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
        { "__index", index_lua },
        { "__newindex", newindex_lua },
        { NULL, NULL }
    };
    struct luaL_Reg *ptr = mmethods;
    
    luaL_newmetatable( L, MODULE_MT );
    // metamethods
    while( ptr->name ){
        lstate_fn2tbl( L, ptr->name, ptr->func );
        ptr++;
    }
    lua_pop( L, 1 );
    
    // module function
    lua_createtable( L, 0, 1 );
    lstate_fn2tbl( L, "new", new_lua );
    
    return 1;
}




