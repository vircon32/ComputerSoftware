int Factorial( int n )
{
    if( n <= 1 ) return 1;
    return Factorial( n - 1 );
}

void main()
{
    Factorial( 3 );
}