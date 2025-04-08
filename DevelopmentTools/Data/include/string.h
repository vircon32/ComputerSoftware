/* *****************************************************************************
*  Vircon32 standard library: "string.h"          File version: 2025/04/08     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all functions needed by the programs to process strings  *
*  and characters in general. This includes character classification, string   *
*  building and comparison, and conversion of numbers to string.               *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef STRING_H
    #define STRING_H
// *****************************************************************************


// =============================================================================
//   CHARACTER CLASSIFICATION FUNCTIONS
// =============================================================================


bool isdigit( int c )
{
    return (c >= '0' && c <= '9');
}

// -----------------------------------------------------------------------------

bool isxdigit( int c )
{
    if( c >= '0' && c <= '9' )
      return true;
    
    if( c >= 'a' && c <= 'f' )
      return true;
  
    return (c >= 'A' && c <= 'F');
}

// -----------------------------------------------------------------------------

bool isalpha( int c )
{
    if( c >= 'a' && c <= 'z' )
      return true;
  
    return (c >= 'A' && c <= 'Z');
}

// -----------------------------------------------------------------------------

bool isascii( int c )
{
    return (c >= 0 && c <= 127);
}

// -----------------------------------------------------------------------------

bool isalphanum( int c )
{
    // avoid calling the other functions
    // to have less calling overhead
    if( c >= '0' && c <= '9' )
      return true;
    
    if( c >= 'a' && c <= 'z' )
      return true;
  
    return (c >= 'A' && c <= 'Z');
}

// -----------------------------------------------------------------------------

bool islower( int c )
{
    // standard ascii
    if( c >= 'a' && c <= 'z' )
      return true;
    
    // extended ascii for code page Windows-1252
    return (c >= 224 && c <= 254 && c != 247);
}

// -----------------------------------------------------------------------------

bool isupper( int c )
{
    // standard ascii
    if( c >= 'A' && c <= 'Z' )
      return true;
    
    // extended ascii for code page Windows-1252
    return (c >= 192 && c <= 222 && c != 215);
}

// -----------------------------------------------------------------------------

bool isspace( int c )
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}


// =============================================================================
//   CHARACTER CONVERSION FUNCTIONS
// =============================================================================


int tolower( int c )
{
    if( !isupper( c ) )
      return c;
    
    return c + 32;
}

// -----------------------------------------------------------------------------

int toupper( int c )
{
    if( !islower( c ) )
      return c;
    
    return c - 32;
}


// =============================================================================
//   STRING HANDLING FUNCTIONS
// =============================================================================


int strlen( int* text )
{
    int* first_position = text;
    
    while( *text )
      ++text;
    
    return text - first_position;
}

// -----------------------------------------------------------------------------

int strcmp( int* text1, int* text2 )
{
    while( *text1 && *text2 )
    {
        if( *text1 != *text2 )
          break;
        
        ++text1;
        ++text2;
    }
    
    return *text1 - *text2;
}

// -----------------------------------------------------------------------------

int strncmp( int* text1, int* text2, int max_characters )
{
    // check for the weird cases
    if( max_characters < 1 )
      return 0;
    
    // now actually compare
    while( *text1 && *text2 )
    {
        // check max characters before anything else
        // or we will do 1 too many, because there
        // is an extra comparison out of the loop
        --max_characters;
        
        if( max_characters <= 0 )
          break;
        
        // otherwise keep comparing
        if( *text1 != *text2 )
          break;
        
        ++text1;
        ++text2;
    }
    
    return *text1 - *text2;
}

// -----------------------------------------------------------------------------

void strcpy( int* dest_text, int* src_text )
{
    while( *src_text )
    {
        *dest_text = *src_text;
        ++dest_text;
        ++src_text;
    }
    
    // add null termination
    *dest_text = 0;
}

// -----------------------------------------------------------------------------

void strncpy( int* dest_text, int* src_text, int max_characters )
{
    // check for the weird cases
    if( max_characters < 1 )
      return;
    
    // now actually copy
    while( *src_text )
    {
        *dest_text = *src_text;
        ++dest_text;
        ++src_text;
        
        // exit early if max characters are reached
        --max_characters;
        
        if( max_characters <= 0 )
          break;
    }
    
    // add null termination
    *dest_text = 0;
}

// -----------------------------------------------------------------------------

// initial text will be modified
// careful! no control of buffer length is done
void strcat( int* initial_text, int* added_text )
{
    // first, find the null termination
    // for the original text
    while( *initial_text )
      ++initial_text;
    
    // now operate just as strcpy
    while( *added_text )
    {
        *initial_text = *added_text;
        ++initial_text;
        ++added_text;
    }
    
    // add null termination
    *initial_text = 0;
}

// -----------------------------------------------------------------------------

void strncat( int* initial_text, int* added_text, int max_characters )
{
    // check for the weird cases
    if( max_characters < 1 )
      return;
    
    // first, find the null termination
    // for the original text
    while( *initial_text )
      ++initial_text;
    
    // now operate just as in strcpy
    while( *added_text )
    {
        *initial_text = *added_text;
        ++initial_text;
        ++added_text;
        
        // exit early if max characters are reached
        --max_characters;
        
        if( max_characters <= 0 )
          break;
    }
    
    // add null termination
    *initial_text = 0;
}


// =============================================================================
//   STRING SEARCH FUNCTIONS
// =============================================================================


// strchr
// strstr


// =============================================================================
//   CONVERSION OF NUMBERS TO STRING
// =============================================================================


// converts an integer number to a null-terminated
// character string, storing it in the given buffer;
// careful! no control of buffer length is done; it
// will accept bases from 2 to 16; only in the case
// of base 10, value is taken as signed
void itoa( int value, int* result_text, int base )
{
    int[ 16+1 ] hex_characters = "0123456789ABCDEF";
    int[ 32+1 ] reversed_digits;
    int* next_digit = reversed_digits;
    
    // do nothing if base is not in range [2-16]
    if( base < 2 || base > 16 )
      return;
    
    // for base 10 numbers, prepend the sign if needed
    if( base == 10 && value < 0 )
    {
        // special treatment for 0x80000000 since we can't
        // change its sign in an int32 (because -2147483648
        // can be represented but +2147483648 cannot)
        if( value == 0x80000000 )
        {
            int[ 11+1 ] int32_min = "-2147483648";
            strcpy( result_text, int32_min );
            return;
        }
        
        *result_text = '-';
        ++result_text;
        value = -value;
    }
    
    // otherwise do a first iteration if the sign bit is set
    // to remove it and treat next iterations as positive
    else if( value < 0 )
    {
        // split value in the sum of 2 parts, to prevent any quantity
        // from reaching the last bit and being taken as negative
        // (thus, we must use 2*0x40000000 instead of 0x80000000!)
        value &= 0x7FFFFFFF;
        int half_complement = 0x40000000;
        int half_complement_mod = 0x40000000 % base;

        // obtaining 1st digit = value % base
        int sum_remainders = 2 * half_complement_mod + value % base;
        int digit_value = sum_remainders % base;
        *next_digit = hex_characters[ digit_value ];
        ++next_digit;

        // this will obtain "original value / base", as if
        // the original value was an unsigned 32-bit integer
        // next value = (original value) / base
        value = 2 * (half_complement / base) + (value / base) + (sum_remainders / base);
        if( !value ) goto digits_stored;
    }
    
    // keep adding digits starting from the right
    do
    {
        *next_digit = hex_characters[ value % base ];
        ++next_digit;
        
        value /= base;
    }
    while( value );
    
    digits_stored:
    
    // now place the digits into result, in the correct order
    do
    {
        --next_digit;
        *result_text = *next_digit;
        ++result_text;
    }
    while( next_digit != &reversed_digits[0] );
    
    // add the null termination
    *result_text = 0;
}

// -----------------------------------------------------------------------------

// careful! no control of buffer length is done
void ftoa( float value, int* result_text )
{
    // first, see if we need to use scientific notation
    // (pending, not supported yet)
    
    // write the negative sign when needed
    // and then treat all numbers as positive
    if( value < 0 )
    {
        *result_text = '-';
        ++result_text;
        value = -value;
    }
    
    // separate the integer and decimal parts
    // (careful: not the same in negative numbers)
    int integer_part, decimal_part;
    
    // this block replaces "integer_part = floor( value )"
    // to avoid dependending from math header
    asm
    {
        "mov R0, {value}"
        "flr R0"
        "cfi R0"
        "mov {integer_part}, R0"
    }
    
    // this block replaces "decimal_part = round( (value - integer_part) * 100000 )"
    // where we take up to 5 significant digits and round
    asm
    {
        "push R1"
        "mov R0, {value}"
        "mov R1, {integer_part}"
        "cif R1"
        "fsub R0, R1"
        "fmul R0, 100000.0"
        "round R0"
        "cfi R0"
        "mov {decimal_part}, R0"
        "pop R1"
    }
    
    // write the integer part using itoa
    itoa( integer_part, result_text, 10 );
    
    // if number is integer, we are finished
    if( !decimal_part )
      return;
    
    // otherwise write decimal separator
    strcat( result_text, "." );
    
    // add leftmost decimal zeroes when needed
    int decimal_aux = decimal_part;
    
    while( decimal_aux < 10000 )
    {
        strcat( result_text, "0" );
        decimal_aux *= 10;
    }
    
    // remove righmost decmal zeroes when needed
    while( !(decimal_part % 10) )
      decimal_part /= 10;
    
    // write the decimals as a number using itoa
    int[ 8 ] decimal_string;
    itoa( decimal_part, decimal_string, 10 );
    strcat( result_text, decimal_string );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
