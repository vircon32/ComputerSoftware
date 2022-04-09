/* *****************************************************************************
*  Vircon32 standard library: "input.h"           File version: 2021/02/18     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all definitions and functions needed by the programs to  *
*  interact with Vircon gamepad controller, and read gamepad input in general. *
*  This includes global gamepad controller parameters, as well as specific     *
*  functions to read the state of directions and buttons.                      *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef INPUT_H
    #define INPUT_H
// *****************************************************************************


// =============================================================================
//   GENERAL GAMEPAD FUNCTIONS
// =============================================================================


void select_gamepad( int gamepad_id )
{
    asm
    {
        "mov R0, {gamepad_id}"
        "out INP_SelectedGamepad, R0"
    }
}

// -----------------------------------------------------------------------------

int get_selected_gamepad()
{
    asm
    {
        "in R0, INP_SelectedGamepad"
    }
}

// -----------------------------------------------------------------------------

// applies to the currently selected gamepad;
// unlike directions and buttons, it is only
// a yes/no value (provides no time information)
bool gamepad_is_connected()
{
    asm
    {
        "in R0, INP_GamepadConnected"
    }
}


// =============================================================================
//   READING D-PAD DIRECTION
// =============================================================================


int gamepad_left()
{
    asm
    {
        "in R0, INP_GamepadLeft"
    }
}

// -----------------------------------------------------------------------------

int gamepad_right()
{
    asm
    {
        "in R0, INP_GamepadRight"
    }
}

// -----------------------------------------------------------------------------

int gamepad_up()
{
    asm
    {
        "in R0, INP_GamepadUp"
    }
}

// -----------------------------------------------------------------------------

int gamepad_down()
{
    asm
    {
        "in R0, INP_GamepadDown"
    }
}

// -----------------------------------------------------------------------------

// movements are integers (form a square)
void gamepad_direction( int* delta_x, int* delta_y )
{
    // read direction in x
    if( gamepad_left()  > 0 )
      *delta_x = -1;
  
    else if( gamepad_right() > 0 )
      *delta_x = 1;
  
    else
      *delta_x = 0;

    // read direction in y
    if( gamepad_up()  > 0 )
      *delta_y = -1;
  
    else if( gamepad_down() > 0 )
      *delta_y = 1;
  
    else
      *delta_y = 0;
}

// -----------------------------------------------------------------------------

// when not 0,0 provides a unit vector (keeps length 1)
void gamepad_direction_normalized( float* delta_x, float* delta_y )
{
    // read direction in x
    if( gamepad_left()  > 0 )
      *delta_x = -1;
  
    else if( gamepad_right() > 0 )
      *delta_x = 1;
  
    else
      *delta_x = 0;

    // read direction in y
    if( gamepad_up()  > 0 )
      *delta_y = -1;
  
    else if( gamepad_down() > 0 )
      *delta_y = 1;
  
    else
      *delta_y = 0;
    
    // normalize diagonals unsing factor sqrt(2)/2
    if( *delta_x && *delta_y )
    {
        *delta_x *= 0.70710678;
        *delta_y *= 0.70710678;
    }
}


// =============================================================================
//   READING GAMEPAD BUTTONS
// =============================================================================


int gamepad_button_a()
{
    asm
    {
        "in R0, INP_GamepadButtonA"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_b()
{
    asm
    {
        "in R0, INP_GamepadButtonB"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_x()
{
    asm
    {
        "in R0, INP_GamepadButtonX"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_y()
{
    asm
    {
        "in R0, INP_GamepadButtonY"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_l()
{
    asm
    {
        "in R0, INP_GamepadButtonL"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_r()
{
    asm
    {
        "in R0, INP_GamepadButtonR"
    }
}

// -----------------------------------------------------------------------------

int gamepad_button_start()
{
    asm
    {
        "in R0, INP_GamepadButtonStart"
    }
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
