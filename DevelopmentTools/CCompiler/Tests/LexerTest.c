#define Width 640
#define MaxX (Width - 40)
#define LimitX ((-2 + MaxX))


void main( void )
{
    int x = LimitX();
    
    if( 100 < (x+(LimitX))) ++x;
}