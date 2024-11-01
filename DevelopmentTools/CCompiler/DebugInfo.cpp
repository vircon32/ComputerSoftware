// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "Globals.hpp"
    #include "DebugInfo.hpp"
    
    // include C/C++ headers
    #include <fstream>      // [ C++ STL ] File streams
    #include <cstring>      // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      FUNCTIONS TO OUTPUT FILES WITH DEBUG INFORMATION
// =============================================================================


void SaveDebugInfoFile( const string& FilePath, const VirconCParser& Parser, const VirconCEmitter& Emitter )
{
    if( VerboseMode )
      cout << "saving debug info file" << endl;
    
    // open output file,
    ofstream DebugInfoFile;
    DebugInfoFile.open( FilePath );
    
    if( DebugInfoFile.fail() )
      throw runtime_error( "cannot open debug info file \"" + FilePath + "\"" );
    
    // PENDING
    
    // close output
    DebugInfoFile.close();
}

// -----------------------------------------------------------------------------

void SaveLexerLog( const string& FilePath, const VirconCPreprocessor& Preprocessor )
{
    if( VerboseMode )
      cout << "Debug mode: Saving lexer log" << endl;
    
    ofstream LogFile;
    LogFile.open( FilePath );
    
    if( LogFile.fail() )
      throw runtime_error( "cannot open lexer log file \"" + FilePath + "\"" );
    
    // log all tokens, with their line numbers
    string PathContext = "";
    
    for( auto T : Preprocessor.ProcessedTokens )
    {
        if( T->Location.FilePath != PathContext )
        {
            LogFile << endl << "File " + T->Location.FilePath + ":" << endl << endl;
            PathContext = T->Location.FilePath;
        }
        
        LogFile << "[" + to_string( T->Location.Line ) + "] ";
        LogFile << T->ToString() << endl;
    }
    
    LogFile.close();
}

// -----------------------------------------------------------------------------

void SaveParserLog( const string& FilePath, const VirconCParser& Parser )
{
    if( VerboseMode )
      cout << "Debug mode: Saving parser log" << endl;
    
    ofstream LogFile;
    LogFile.open( FilePath );
    
    if( LogFile.fail() )
      throw runtime_error( "cannot open parser log file \"" + FilePath + "\"" );
    
    // log full AST tree in XML format
    LogFile << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
    LogFile << Parser.ProgramAST->ToXML();
    
    LogFile.close();
}
