#define BubblesInX  16
#define BubblesInY  12

int[ BubblesInY ][ BubblesInX ] Bubbles;

void main()
{
    int* B = &Bubbles[0][0];
    
    B[1] = 9;
    
    *(B+2) = 11;
    
    *(--B) = 13;
    
    *(B++) = 15;
    
    (B+=17) = &Bubbles[0][0];
}