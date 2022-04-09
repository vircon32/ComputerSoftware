
struct Point
{
    int x,y;
};

struct Box
{
    Point TopLeft;
    Point BottomRight;
};

void main()
{
    Box B;
    int TopY = B.TopLeft.y;
    
    Box* PointedBox = &B;
    int LeftX = PointedBox->TopLeft.x;
    int RightX = (*PointedBox).BottomRight.x;
}