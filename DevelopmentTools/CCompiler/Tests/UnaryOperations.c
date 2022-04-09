// ---------------------------------------------------------
//   SCENE DEFINITION
// ---------------------------------------------------------

struct Point
{
    int x,y;
};


// ---------------------------------------------------------
//   MAIN FUNCTION
// ---------------------------------------------------------


// notice that main prototype is different
// in Vircon since there is no other program
void main( void )
{   
    Point P;
    ~P;
    P++;
    P--;
    !P;
    *P;
}
