
struct Point
{
    int x,y;
};
    
void main()
{
    Point P;
    P.x = 10;
    P.y = -P.x;
    
    Point P1, P2;
    P1 = P2;
    P1 == P2;
    
    int PointedX = (&P)->x;
}