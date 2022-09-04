// partial definitions
bool somefunction();
struct s;
union u;
enum e;

// repeat partial definitions
bool somefunction();
struct s;
union u;
enum e;

// now, full definitions
bool somefunction() { return false; }
struct s{ bool Alpha; };
union u{ int Beta; };
enum e{ Gamma=1 };

// partial definitions can keep appearing
bool somefunction();
struct s;
union u;
enum e;

void main( void )
{
    int a = somefunction();
    s test_s;
    u test_u;
    e test_e;
}