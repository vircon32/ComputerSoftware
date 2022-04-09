
void main( void )
{
    enum Colors
    {
        Red,
        Green,
        Blue
    };
    
    Colors R = Red;
    
    switch( R )
    {
        case Red: break;
        case Green:
        case Blue:
        //default:
    }
}
