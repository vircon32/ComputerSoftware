// *****************************************************************************
    // include project headers
    #include "VirconDefinitions.hpp"
    #include "VirconEnumerations.hpp"
    
    // include C/C++ headers
    #include <sstream>      // [ C++ STL ] String streams
    #include <iomanip>      // [ C++ STL ] I/O Manipulation
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      NUMBER FORMATTING FUNCTIONS
// =============================================================================


string Hex( uint32_t Value, int Digits )
{
    std::stringstream ss;
    ss << nouppercase << "0x";
    ss << uppercase << hex << setfill('0') << setw(Digits);
    ss << Value;

    return ss.str();
}

// -----------------------------------------------------------------------------

string Bin( uint32_t Value, int Digits )
{
    std::stringstream ss;
    
    for( int Bit = Digits-1; Bit >= 0; Bit-- )
    {
        unsigned Mask = 1 << Bit;
        bool HasBit = (Value & Mask);
        ss << (HasBit? '1' : '0');
    }
    
    return ss.str();
}
