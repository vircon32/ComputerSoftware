
union Word
{
    int AsInt;
    float AsFloat;
    bool AsBool;
};

struct Point
{
    int x,y;
};

union MultiWord
{
    Point P;
    int[4] Array;
    Word[2] Words;
};

void main()
{
    Word W;
    W.AsInt = 10;
    W.AsFloat = -W.AsInt;
    
    Word W1, W2;
    W1 = W2;
    
    int s = sizeof( MultiWord );
}