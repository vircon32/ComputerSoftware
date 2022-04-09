struct S
{
    S* SelfRef;
};

union U
{
    U* SelfRef;
};

void main()
{
    S S1;
    U U1;
}