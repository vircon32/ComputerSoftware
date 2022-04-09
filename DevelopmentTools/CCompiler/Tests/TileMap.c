#include "time.h"
#include "video.h"
#include "input.h"
#include "math.h"


// ---------------------------------------------------------
//   DEFINITIONS AND CONSTANTS
// ---------------------------------------------------------


// names for texture regions
#define FirstRegionTileSet     0
#define RegionArrowLeft      100
#define RegionArrowRight     101
#define RegionArrowUp        102
#define RegionArrowDown      103

// screen dimensions
#define ScreenWidth   640
#define ScreenHeight  360

// tile map dimensions (2x2 screens)
#define MapTilesX  32
#define MapTilesY  18
#define TileSize   40

// position of map limits, in pixels
#define LevelMinX  0
#define LevelMinY  0
#define LevelMaxX  (MapTilesX * TileSize)
#define LevelMaxY  (MapTilesY * TileSize)

// precalculate camera limits
#define CameraMinX (LevelMinX + ScreenWidth/2)
#define CameraMaxX (LevelMaxX - ScreenWidth/2)
#define CameraMinY (LevelMinY + ScreenHeight/2)
#define CameraMaxY (LevelMaxY - ScreenHeight/2)


// ------
//   GLOBAL VARIABLES
// ---------------------------------------------------------


// external variables in rom
embedded int[ MapTilesY ][ MapTilesX ] TileMap = "TileMap.data";


// ---------------------------------------------------------
//   MAIN FUNCTION
// ---------------------------------------------------------


void main( void )
{
    // these selections will never change
    select_texture( 0 );
    select_gamepad( 0 );
    
    // ------------------------------------
    // PART 1: DEFINE ALL TEXTURE REGIONS
    // ------------------------------------
    
    // regions for tileset
    define_region_matrix( FirstRegionTileSet,  0,0,  40,40,  8,6,  0 );
    
    // left arrow
    select_region( RegionArrowLeft );
    define_region( 0,241,  15,272,  0,256 );
    
    // right arrow
    select_region( RegionArrowRight );
    define_region( 17,241,  32,272,  32,256 );
    
    // up arrow
    select_region( RegionArrowUp );
    define_region( 34,241,  65,256,  49,241 );
    
    // down arrow
    select_region( RegionArrowDown );
    define_region( 34,258,  65,273,  49,273 );
    
    // ------------------------------------
    // PART 2: PREPARATIONS
    // ------------------------------------
    
    int CameraX = LevelMaxX / 2;
    int CameraY = LevelMaxY / 2;
    
    // ------------------------------------
    // PART 3: MAIN LOOP
    // ------------------------------------
    
    while( true )
    {
        // read inputs
        select_gamepad( 0 );
        
        // move camera as indicated by player
        int DeltaX, DeltaY;
        gamepad_direction( &DeltaX, &DeltaY );
        
        CameraX += 2 * DeltaX;
        CameraY += 2 * DeltaY;
        
        // keep camera within stage limits
        CameraX = max( CameraX, CameraMinX );
        CameraX = min( CameraX, CameraMaxX );
        CameraY = max( CameraY, CameraMinY );
        CameraY = min( CameraY, CameraMaxY );
        
    }
}