int add( int a, int b )
{
    return a + b;
}

int s1, s2, s3;

void main()
{
    int( int, int )* fptr = &add;
    s1 = add( 10, 20 );
    s2 = fptr( 30, 40 );
    s3 = (*fptr)( 50, 60 );
}