// we actually need to write argument names
// (while in standard C only types are required)
int F1( int Number );

int F2()
{
    return 2*F1( 5 );
}

// argument name is different on purpose
// (it is only required to math the definition's types)
int F1( int Parameter )
{
    return Parameter;
}

void main()
{
    int a = F2();
}