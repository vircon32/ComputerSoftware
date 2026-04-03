int add( int a, int b );
int sub( int a, int b );

int(int, int)*[ 2 ] operations = { &add, &sub };
int sum;

void main()
{
    sum = operations[ 0 ]( 10, 20 );
}

int add( int a, int b )
{
    return a + b;
}

int sub( int a, int b )
{
    return a - b;
}
