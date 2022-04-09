void main()
{
    struct Test;
    
    Test* TPointer = NULL;
    
    struct Test
    {
        int a,b;
        Test* Next;
    };
    
    Test T;
    TPointer = &T;
}
