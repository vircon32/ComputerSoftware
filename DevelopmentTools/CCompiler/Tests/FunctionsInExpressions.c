int Two()
{
    return 2;
}

int Four()
{
    Two() + Two();
}

int Double( int n )
{
    return 2 * n;
}

int Twelve()
{
    int a = Double( Two() );
    return 3 * a;
}

void Empty()
{}

// ---------------------------------------------------------

void main( void )
{   
    int a = Two() + (Four()*3);
}
