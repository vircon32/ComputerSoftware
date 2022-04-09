enum Numbers
{
    Zero,
    One,
    Two,
    Three
};

void main( void )
{
    enum Parity
    {
        Odd = 0,
        Even = Odd + 1
    };
    
    Parity P = Even, Q = Odd;
    
    int i = P;
    int j = ~Odd + 1;
    
    P < Q;
    P == Odd;
    P != Odd;
    P < Odd;
    P >= (Odd + Three);
    
    int n = Two;
}
