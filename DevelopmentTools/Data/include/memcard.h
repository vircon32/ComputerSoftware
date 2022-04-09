/* *****************************************************************************
*  Vircon32 standard library: "memcard.h"         File version: 2021/02/18     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file allows programs to detect and use Vircon memory cards. Includes   *
*  functions to identify card contents and for general read/write access.      *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef MEMCARD_H
    #define MEMCARD_H
// *****************************************************************************


// =============================================================================
//   DETECTING A MEMORY CARD
// =============================================================================


bool card_is_connected()
{
    asm
    {
        "in R0, MEM_Connected"
    }
}


// =============================================================================
//   MEMORY CARD SIGNATURES
// =============================================================================


// this will be used in several functions
typedef int[20] game_signature;

// -----------------------------------------------------------------------------

void card_read_signature( game_signature* signature )
{
    asm
    {
        "mov CR, 20"
        "mov DR, {signature}"
        "mov SR, 0x30000000"
        "movs"
    }
}

// -----------------------------------------------------------------------------

void card_write_signature( game_signature* signature )
{
    asm
    {
        "mov CR, 20"
        "mov DR, 0x30000000"
        "mov SR, {signature}"
        "movs"
    }
}

// -----------------------------------------------------------------------------

bool card_signature_matches( game_signature* expected_signature )
{
    asm
    {
        "mov CR, 20"
        "mov DR, {expected_signature}"
        "mov SR, 0x30000000"
        "cmps R0"
        "bnot R0"
    }
}

// -----------------------------------------------------------------------------

bool card_is_empty()
{
    // vircon programs may assume that a memory card
    // that has only zeroes in its signature is empty
    game_signature zeroes;
    int* first_zero = zeroes;
    
    // fill signature with zeroes
    asm
    {
        "mov CR, 20"
        "mov DR, {first_zero}"
        "mov SR, 0"
        "sets"
    }
    
    // compare card signature
    return card_signature_matches( &zeroes );
}


// =============================================================================
//   GENERAL ACCESS TO THE CARD
// =============================================================================


// these work similar to memcpy
void card_read_data( void* destination, int offset_in_card, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, {destination}"
        "mov SR, 0x30000000"
        "mov R0, {offset_in_card}"
        "iadd SR, R0"
        "movs"
    }
}

// -----------------------------------------------------------------------------

void card_write_data( void* source, int offset_in_card, int size )
{
    asm
    {
        "mov CR, {size}"
        "mov DR, 0x30000000"
        "mov R0, {offset_in_card}"
        "iadd DR, R0"
        "mov SR, {source}"
        "movs"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
