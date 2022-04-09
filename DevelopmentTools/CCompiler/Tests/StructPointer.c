struct Vector2D
{
    float x,y;
};

struct Ray
{
    Vector2D Start, End;
    bool Active;
};

Ray[3] AimTrajectory;

void main()
{
    Ray* CurrentRay = &AimTrajectory[ 0 ];
    
    CurrentRay++;
    CurrentRay--;
    ++CurrentRay;
    --CurrentRay;
    
    CurrentRay+10;
    CurrentRay-10;
}