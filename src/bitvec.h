/*
 *  Copyright 2013-2015 Masatoshi Teruya. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to 
 *  deal in the Software without restriction, including without limitation the 
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 *  bitvec.h
 *  Created by Masatoshi Teruya on 13/12/25.
 *
 */

#ifndef ___BITVEC_H___
#define ___BITVEC_H___

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#define BV_TYPE uint64_t
static const uint8_t BV_BIT = CHAR_BIT * sizeof( BV_TYPE );

typedef struct {
    size_t nbit;
    size_t nvec;
    BV_TYPE *vec;
} bitvec_t;

#define BIT2VEC_SIZE(nbit) \
    (nbit < BV_BIT ? 1 :( nbit / BV_BIT ) + !!( nbit % BV_BIT ))

static inline int bitvec_init( bitvec_t *bv, size_t nbit )
{
    bv->nbit = nbit;
    bv->nvec = BIT2VEC_SIZE( nbit );
    if( ( bv->vec = calloc( bv->nvec, sizeof( BV_TYPE ) ) ) ){
        return 0;
    }
    bv->vec = NULL;
    
    return -1;
}

static inline int bitvec_resize( bitvec_t *bv, size_t nbit )
{
    if( nbit == bv->nbit ){
        return 0;
    }
    else
    {
        size_t nvec = BIT2VEC_SIZE( nbit );
        BV_TYPE *vec = bv->vec;
        
        // mem error
        if( nvec != bv->nvec && 
            !( vec = realloc( vec, sizeof( BV_TYPE ) * nvec ) ) ){
            return -1;
        }
        
        // clear allocated bits
        if( nvec > bv->nvec ){
            memset( vec + bv->nvec, 0, ( nvec - bv->nvec ) * sizeof( BV_TYPE ) );
        }
        // clear unused bits
        else {
            vec[nvec - 1] &= ( ~((BV_TYPE)0) >> ( BV_BIT * nvec - nbit - 1 ) );
        }
        
        bv->vec = vec;
        bv->nvec = nvec;
        bv->nbit = nbit;
        
        return 0;
    }
}

#undef BIT2VEC_SIZE


static inline void bitvec_dispose( bitvec_t *bv )
{
    if( bv->vec ){
        free( bv->vec );
        memset( (void*)bv, 0, sizeof( bitvec_t ) );
    }
}


static inline int bitvec_get( bitvec_t *bv, BV_TYPE pos )
{
    if( bv->vec && pos <= bv->nbit ){
        return (int)((bv->vec[pos/BV_BIT] >> (pos % BV_BIT)) & (BV_TYPE)1);
    }
    
    return -1;
}


static inline int bitvec_set( bitvec_t *bv, BV_TYPE pos )
{
    if( bv->vec && pos <= bv->nbit ){
        bv->vec[pos / BV_BIT] |= (BV_TYPE)1 << ( pos % BV_BIT );
        return 0;
    }
    
    return -1;
}


static inline int bitvec_unset( bitvec_t *bv, BV_TYPE pos )
{
    if( bv->vec && pos <= bv->nbit ){
        bv->vec[pos / BV_BIT] &= ~( (BV_TYPE)1 << ( pos % BV_BIT ) );
        return 0;
    }
    
    return -1;
}


static inline int bitvec_set_range( bitvec_t *bv, BV_TYPE from, BV_TYPE to )
{
    if( bv->vec && to <= bv->nbit )
    {
        BV_TYPE *vec = bv->vec;
        size_t start = from / BV_BIT;
        size_t end = to / BV_BIT;
        
        if( start == end ){
            vec[start] |= (~( (~(BV_TYPE)1) << ( to - from ) ) << from);
        }
        else {
            size_t pos = start + 1;
            
            for(; pos < end; pos++ ){
                vec[pos] |= ~(BV_TYPE)0;
            }
            vec[start] |= (~(BV_TYPE)0) << ( from % BV_BIT );
            vec[end] |= ~((~(BV_TYPE)1) << ( to % BV_BIT ));
        }
        
        return 0;
    }
    
    return -1;
}


static inline int bitvec_unset_range( bitvec_t *bv, BV_TYPE from, BV_TYPE to )
{
    if( bv->vec && to <= bv->nbit )
    {
        BV_TYPE *vec = bv->vec;
        size_t start = from / BV_BIT;
        size_t end = to / BV_BIT;
        
        if( start == end ){
            vec[start] &= ~(~( (~(BV_TYPE)1) << ( to - from ) ) << from);
        }
        else {
            size_t pos = start + 1;
            
            for(; pos < end; pos++ ){
                vec[pos] &= (BV_TYPE)0;
            }
            vec[start] &= ~((~(BV_TYPE)0) << ( from % BV_BIT ));
            vec[end] &= ~(BV_TYPE)1 << ( to % BV_BIT );
        }
        
        return 0;
    }
    
    return -1;
}


#endif

