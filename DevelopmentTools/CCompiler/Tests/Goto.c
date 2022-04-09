//middle: // wrong

void main( void )
{
    start:
    
    if( true )
    {
        goto end;
        goto start;
    }
    
    end:
    return;
}
