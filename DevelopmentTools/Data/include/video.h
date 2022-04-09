/* *****************************************************************************
*  Vircon32 standard library: "video.h"           File version: 2022/02/25     *
*  --------------------------------------------------------------------------- *
*  This header is part of the Vircon32 C programming tools                     *
*  --------------------------------------------------------------------------- *
*  This file contains all definitions and functions needed by the programs to  *
*  interact with Vircon GPU and draw to the screen in general. This includes   *
*  all global GPU configuration, as well as specific functions for textures    *
*  and texture regions.                                                        *
***************************************************************************** */

// *****************************************************************************
    // start include guard
    #ifndef VIDEO_H
    #define VIDEO_H
// *****************************************************************************


// =============================================================================
//   GENERAL VIRCON DEFINITIONS
// =============================================================================


// properties of the video display
#define screen_width       640
#define screen_height      360


// =============================================================================
//   RGBA COLOR REPRESENTATION
// =============================================================================


// colors are given by their RGBA value,
// but since Vircon's in-word byte order
// is little endian, their representation
// in written form actually has to be ABGR
// (same as in most current computers)
#define color_black      0xFF000000
#define color_white      0xFFFFFFFF
#define color_gray       0xFF808080
#define color_darkgray   0xFF404040
#define color_lightgray  0xFFC0C0C0
#define color_red        0xFF0000FF
#define color_green      0xFF00FF00
#define color_blue       0xFFFF0000
#define color_yellow     0xFF00FFFF
#define color_magenta    0xFFFF00FF
#define color_cyan       0xFFFFFF00
#define color_orange     0xFF0080FF
#define color_brown      0xFF204080

// -----------------------------------------------------------------------------

// assumes full opacity for alpha
// valid range is 0 (black) to 255 (white)
int make_gray( int brightness )
{
    return 0xFF000000 | (brightness << 16) | (brightness << 8) | brightness;
}

// -----------------------------------------------------------------------------

// assumes full opacity for alpha
// valid ranges are [0-255]
int make_color_rgb( int r, int g, int b )
{
    return 0xFF000000 | (b << 16) | (g << 8) | r;
}

// -----------------------------------------------------------------------------

// valid ranges are [0-255]
int make_color_rgba( int r, int g, int b, int a )
{
    return (a << 24) | (b << 16) | (g << 8) | r;
}

// -----------------------------------------------------------------------------

// returned range is [0-255]
int get_color_red( int color )
{
    return color & 255;
}
// -----------------------------------------------------------------------------

int get_color_green( int color )
{
    return (color >> 8) & 255;
}

// -----------------------------------------------------------------------------

int get_color_blue( int color )
{
    return (color >> 16) & 255;
}

// -----------------------------------------------------------------------------

int get_color_alpha( int color )
{
    return (color >> 24) & 255;
}


// =============================================================================
//   FUNCTIONS FOR GPU SELECTED ELEMENTS
// =============================================================================


// texture ids in cartridge start from 0, but
// the BIOS texture at id = -1 is also selectable
void select_texture( int texture_id )
{
    asm
    {
        "mov R0, {texture_id}"
        "out GPU_SelectedTexture, R0"
    }
}

// -----------------------------------------------------------------------------

int get_selected_texture()
{
    asm
    {
        "in R0, GPU_SelectedTexture"
    }
}

// -----------------------------------------------------------------------------

// region id refers to the currently selected texture
void select_region( int region_id )
{
    asm
    {
        "mov R0, {region_id}"
        "out GPU_SelectedRegion, R0"
    }
}

// -----------------------------------------------------------------------------

int get_selected_region()
{
    asm
    {
        "in R0, GPU_SelectedRegion"
    }
}


// =============================================================================
//   DEFINITION OF GPU TEXTURE REGIONS
// =============================================================================


// applies to the currently selected region
void define_region( int min_x, int min_y, int max_x, int max_y, int hotspot_x, int hotspot_y )
{
    asm
    {
        "mov R0, {min_x}"
        "out GPU_RegionMinX, R0"
        "mov R0, {min_y}"
        "out GPU_RegionMinY, R0"
        "mov R0, {max_x}"
        "out GPU_RegionMaxX, R0"
        "mov R0, {max_y}"
        "out GPU_RegionMaxY, R0"
        "mov R0, {hotspot_x}"
        "out GPU_RegionHotSpotX, R0"
        "mov R0, {hotspot_y}"
        "out GPU_RegionHotSpotY, R0"
    }
}

// -----------------------------------------------------------------------------

// applies to the currently selected region;
// sets the hostpot at its top left, i.e. (min_x, min_y)
void define_region_topleft( int min_x, int min_y, int max_x, int max_y )
{
    asm
    {
        "mov R0, {min_x}"
        "out GPU_RegionMinX, R0"
        "out GPU_RegionHotSpotX, R0"
        "mov R0, {min_y}"
        "out GPU_RegionMinY, R0"
        "out GPU_RegionHotSpotY, R0"
        "mov R0, {max_x}"
        "out GPU_RegionMaxX, R0"
        "mov R0, {max_y}"
        "out GPU_RegionMaxY, R0"
    }
}

// -----------------------------------------------------------------------------

// applies to the currently selected region;
// sets the hostpot at its center
void define_region_center( int min_x, int min_y, int max_x, int max_y )
{
    int center_x = (min_x + max_x) / 2;
    int center_y = (min_y + max_y) / 2;
    
    asm
    {
        "mov R0, {min_x}"
        "out GPU_RegionMinX, R0"
        "mov R0, {min_y}"
        "out GPU_RegionMinY, R0"
        "mov R0, {max_x}"
        "out GPU_RegionMaxX, R0"
        "mov R0, {max_y}"
        "out GPU_RegionMaxY, R0"
        "mov R0, {center_x}"
        "out GPU_RegionHotSpotX, R0"
        "mov R0, {center_y}"
        "out GPU_RegionHotSpotY, R0"
    }
}

// -----------------------------------------------------------------------------

// defines a set regions with consecutive ids from first_id;
// these regions are laid out in a rectangular matrix, and
// all have the same size; the coordinates provided are for
// the top-left region, and the hotspots are all placed at
// the same relative point within each region; the gap
// between regions is assumed to be the same in x and y
void define_region_matrix
(
    int first_id,
    int first_min_x, int first_min_y,
    int first_max_x, int first_max_y,
    int first_hotspot_x, int first_hotspot_y,
    int elements_x, int elements_y,
    int gap
)
{
    // init iteration variables
    int current_id = first_id;
    int min_x = first_min_x;
    int min_y = first_min_y;
    int max_x = first_max_x;
    int max_y = first_max_y;
    int hotspot_x = first_hotspot_x;
    int hotspot_y = first_hotspot_y;
    
    // prevalculate advances
    int advance_x = (max_x - min_x + 1) + gap;
    int advance_y = (max_y - min_y + 1) + gap;
    
    // loop elements in y and x
    for( int matrix_y = 0; matrix_y < elements_y; ++matrix_y )
    {
        for( int matrix_x = 0; matrix_x < elements_x; ++matrix_x )
        {
            // define this region
            select_region( current_id );
            define_region( min_x, min_y, max_x, max_y, hotspot_x, hotspot_y );
            current_id++;
            
            // advance in X
            min_x += advance_x;
            max_x += advance_x;
            hotspot_x += advance_x;
        }
        
        // advance in Y
        min_y += advance_y;
        max_y += advance_y;
        hotspot_y += advance_y;
        
        // reset X to left side
        min_x = first_min_x;
        max_x = first_max_x;
        hotspot_x = first_hotspot_x;
    }
}

// -----------------------------------------------------------------------------

// sets the hotspot of the currently selected region
void set_region_hotspot( int hotspot_x, int hotspot_y )
{
    asm
    {
        "mov R0, {hotspot_x}"
        "out GPU_RegionHotSpotX, R0"
        "mov R0, {hotspot_y}"
        "out GPU_RegionHotSpotY, R0"
    }
}


// =============================================================================
//   SETTING GPU PARAMETERS
// =============================================================================


// the effect of this color is the same as in
// OpenGL's glColor (white is neutral)
void set_multiply_color( int color )
{
    asm
    {
        "mov R0, {color}"
        "out GPU_MultiplyColor, R0"
    }
}

// -----------------------------------------------------------------------------

// regions will be drawn so that their hot spot
// is placed at the current drawing point on screen
void set_drawing_point( int drawing_x, int drawing_y )
{
    asm
    {
        "mov R0, {drawing_x}"
        "out GPU_DrawingPointX, R0"
        "mov R0, {drawing_y}"
        "out GPU_DrawingPointY, R0"
    }
}

// -----------------------------------------------------------------------------

// negative scale factors are allowed, which have
// the effect of flipping the image along x and y
void set_drawing_scale( float scale_x, float scale_y )
{
    asm
    {
        "mov R0, {scale_x}"
        "out GPU_DrawingScaleX, R0"
        "mov R0, {scale_y}"
        "out GPU_DrawingScaleY, R0"
    }
}

// -----------------------------------------------------------------------------

// angle is given in radians, and because screen y
// coordinates grow downwards (origin is in top-left),
// growing angles mean rotating clockwise
void set_drawing_angle( float angle )
{
    asm
    {
        "mov R0, {angle}"
        "out GPU_DrawingAngle, R0"
    }
}

// -----------------------------------------------------------------------------

// available blending modes
#define blending_alpha     0x20
#define blending_add       0x21
#define blending_subtract  0x22

// the blending mode set will continue to be active
// for all operations until a different mode is set
void set_blending_mode( int mode )
{
    asm
    {
        "mov R0, {mode}"
        "out GPU_ActiveBlending, R0"
    }
}


// =============================================================================
//   GETTING GPU PARAMETERS
// =============================================================================


int get_multiply_color()
{
    asm
    {
        "in R0, GPU_MultiplyColor"
    }
}

// -----------------------------------------------------------------------------

void get_drawing_point( int* drawing_x, int* drawing_y )
{
    asm
    {
        "push R1"
        "in R0, GPU_DrawingPointX"
        "mov R1, {drawing_x}"
        "mov [R1], R0"
        "in R0, GPU_DrawingPointY"
        "mov R1, {drawing_y}"
        "mov [R1], R0"
    }
}

// -----------------------------------------------------------------------------

void get_drawing_scale( float* scale_x, float* scale_y )
{
    asm
    {
        "push R1"
        "in R0, GPU_DrawingScaleX"
        "mov R1, {scale_x}"
        "mov [R1], R0"
        "in R0, GPU_DrawingScaleY"
        "mov R1, {scale_y}"
        "mov [R1], R0"
    }
}

// -----------------------------------------------------------------------------

float get_drawing_angle()
{
    asm
    {
        "in R0, GPU_DrawingAngle"
    }
}

// -----------------------------------------------------------------------------

int get_blending_mode()
{
    asm
    {
        "in R0, GPU_ActiveBlending"
    }
}


// =============================================================================
//   REQUESTING GPU COMMANDS
// =============================================================================


void clear_screen( int color )
{
    asm
    {
        "mov R0, {color}"
        "out GPU_ClearColor, R0"
        "out GPU_Command, GPUCommand_ClearScreen"
    }
}

// -----------------------------------------------------------------------------

// draws the currently selected region
// uses the current drawing point
void draw_region()
{
    asm
    {
        "out GPU_Command, GPUCommand_DrawRegion"
    }
}

// -----------------------------------------------------------------------------

// draws the currently selected region
// version that specifies drawing point
void draw_region_at( int drawing_x, int drawing_y )
{
    asm
    {
        "mov R0, {drawing_x}"
        "out GPU_DrawingPointX, R0"
        "mov R0, {drawing_y}"
        "out GPU_DrawingPointY, R0"
        "out GPU_Command, GPUCommand_DrawRegion"
    }
}

// -----------------------------------------------------------------------------

// uses the current drawing point and scale
void draw_region_zoomed()
{
    asm
    {
        "out GPU_Command, GPUCommand_DrawRegionZoomed"
    }
}

// -----------------------------------------------------------------------------

// version that specifies drawing point
void draw_region_zoomed_at( int drawing_x, int drawing_y )
{
    asm
    {
        "mov R0, {drawing_x}"
        "out GPU_DrawingPointX, R0"
        "mov R0, {drawing_y}"
        "out GPU_DrawingPointY, R0"
        "out GPU_Command, GPUCommand_DrawRegionZoomed"
    }
}

// -----------------------------------------------------------------------------

// uses the current drawing point and angle
void draw_region_rotated()
{
    asm
    {
        "out GPU_Command, GPUCommand_DrawRegionRotated"
    }
}

// -----------------------------------------------------------------------------

// version that specifies drawing point
void draw_region_rotated_at( int drawing_x, int drawing_y )
{
    asm
    {
        "mov R0, {drawing_x}"
        "out GPU_DrawingPointX, R0"
        "mov R0, {drawing_y}"
        "out GPU_DrawingPointY, R0"
        "out GPU_Command, GPUCommand_DrawRegionRotated"
    }
}

// -----------------------------------------------------------------------------

// uses the current drawing point, scale and angle
void draw_region_rotozoomed()
{
    asm
    {
        "out GPU_Command, GPUCommand_DrawRegionRotozoomed"
    }
}

// -----------------------------------------------------------------------------

// version that specifies drawing point
void draw_region_rotozoomed_at( int drawing_x, int drawing_y )
{
    asm
    {
        "mov R0, {drawing_x}"
        "out GPU_DrawingPointX, R0"
        "mov R0, {drawing_y}"
        "out GPU_DrawingPointY, R0"
        "out GPU_Command, GPUCommand_DrawRegionRotozoomed"
    }
}


// =============================================================================
//   PRINTING TEXT ON SCREEN
// =============================================================================


// properties of the bios font
#define bios_character_width  10
#define bios_character_height 20

// -----------------------------------------------------------------------------

// assumes that the text string is terminated with a 0;
// careful! this function will change the selected region;
// text is drawn with reference on its top-left corner,
// and any '\n' character will result in a new line
void print_at( int drawing_x, int drawing_y, int* text )
{
    // preserve previous texture selection (otherwise,
    // bios texture will be selected after this function)
    int previous_texture = get_selected_texture();
    select_texture( -1 );
    
    // begin drawing characters at the given position
    int initial_drawing_x = drawing_x;
    
    while( *text )
    {
        
        // print this character
        select_region( *text );
        draw_region_at( drawing_x, drawing_y );
        
        // advance in x
        drawing_x += bios_character_width;
        
        // execute line breaks
        if( *text == '\n' )
        {
            // y advances, x is reset
            drawing_x = initial_drawing_x;
            drawing_y += bios_character_height;
        }
        
        // go to next character
        ++text;
    }
    
    // restore previous texture selection
    select_texture( previous_texture );
}

// -----------------------------------------------------------------------------

// simplified version that prints at current drawing point
void print( int* text )
{
    int drawing_x, drawing_y;
    get_drawing_point( &drawing_x, &drawing_y );
    
    print_at( drawing_x, drawing_y, text );
}


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
