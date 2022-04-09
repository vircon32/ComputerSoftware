int Global1, Global2;

void main( void )
{
    int Local1, Local2;
    
    {
        int Local1, Local2, Local3;
        Local3 = Local1 + Local2;
    }
    
    {
        int LocalA;
        LocalA = 17;
    }
    
    Local2 = Local1;
    ++Global1;
}
