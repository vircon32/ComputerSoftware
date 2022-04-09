struct Vector2D
{
    int x,y;
};

struct Box
{
    Vector2D Position;
    int Width, Height;
    int[5] String;
};

// global initializations
Vector2D VG = { 1, 2 };
int[ 10 ] StringG = "Global";

int rand()
{
    return 7;
}

void main()
{
    int a = 14;
    int b = 7 + rand();
    Vector2D V = { 1, 2 };
    Box B = { {3,4}, 20, 15, "Hola" };
    
    Vector2D V2 = V;
    Box B2 = { V2, 40, 30, "Hi" };
    
    int[ 4 ] i = { 1,2,3,4 };
    
    Vector2D VRand = { rand(), rand() };
    
    // nested lists to fill a 2D array
    int[2][3] Array2D = { {4,5,6}, {7,8,9} };
}