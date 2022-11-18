// *****************************************************************************
    // include project headers
    #include "UserActions.hpp"
    #include "VirconEmulator.hpp"
    #include "Globals.hpp"
    
    // include C/C++ headers
    #include <iostream>           // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


namespace UserActions
{
    void LoadCartridge()
    {
        // console needs to be off
        if( Vircon.PowerIsOn )
        {
            cout << "Cannot load a cartridge when the console is on" << endl;
            return;
        }
        
        // there needs to be no cartridge
        if( Vircon.HasCartridge() )
        {
            cout << "Cannot load a cartridge, remove the previous one first" << endl;
            return;
        }
        
        // request a file path for the ROM
        string EnteredPath;
        cout << "Enter a file path for the cartridge ROM to load:" << endl;
        cin >> EnteredPath;
        
        // attempt to load the cartridge
        Vircon.LoadCartridge( EnteredPath );
        
        // automatically power on
        Vircon.PowerOn();
    }
    
    // -----------------------------------------------------------------------------
    
    void UnloadCartridge()
    {
        // console needs to be off
        if( Vircon.PowerIsOn )
        {
            cout << "Cannot unload a cartridge when the console is on" << endl;
            return;
        }
        
        // attempt to unload the ROM
        Vircon.UnloadCartridge();
    }
    
    // -----------------------------------------------------------------------------
    
    void LoadMemoryCard()
    {
        // there needs to be no card
        if( Vircon.HasMemoryCard() )
        {
            cout << "Cannot load a memory card, remove the previous one first" << endl;
            return;
        }
        
        // request a file path for the ROM
        string EnteredPath;
        cout << "Enter a file path for the memory card to load:" << endl;
        cin >> EnteredPath;
        
        // attempt to load the cartridge
        Vircon.LoadMemoryCard( EnteredPath );
    }
    
    // -----------------------------------------------------------------------------
    
    void UnloadMemoryCard()
    {
        // attempt to unload the memory card
        Vircon.UnloadMemoryCard();
    }
}
