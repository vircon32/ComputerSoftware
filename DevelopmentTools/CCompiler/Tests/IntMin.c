#define INT_MIN 0x80000000

void main()
{
    // 3 correct ways to write INT_MIN
    int a = 0x80000000;
    int b = -2147483647 - 1;
    int c = INT_MIN;
    
    // but this way gives a compiler error (number out of range)
    //int d = -2147483648;
}

