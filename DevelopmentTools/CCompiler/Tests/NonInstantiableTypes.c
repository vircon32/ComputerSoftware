void OKFunction( void )
{}

void[4] MyFunction( void() param1 )
{}


struct s;
union u;

struct s
{
    void v;
    void* v_ptr;
};

union u
{
    void[3] v3;
};

void main()
{
    void v;
    void() vf;
    void()* vf_ptr;
}