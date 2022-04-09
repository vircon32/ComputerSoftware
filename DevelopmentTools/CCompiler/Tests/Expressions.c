void main()
{
    //2 * 3 * 4;        // parseado como: (2*3) * 4   --> correcto
    // 1 + 2 * 3 / 4;   // ANTES: parseado como: 1 + 2*(3/4) --> incorrecto
    
    // caso mas complejo:
    // debe parsearse como:   equal( shift( 0, subtract( add(1,2), divide( multiply(3,4), 5 ))), -1 )
    0 << 1 + 2 - 3 * 4 / 5 == -1;
}