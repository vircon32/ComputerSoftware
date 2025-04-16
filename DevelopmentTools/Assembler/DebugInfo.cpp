// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    #include "../DevToolsInfrastructure/StringFunctions.hpp"
    
    // include project headers
    #include "Globals.hpp"
    #include "DebugInfo.hpp"
    
    // include C/C++ headers
    #include <fstream>      // [ C++ STL ] File streams
    #include <algorithm>    // [ C++ STL ] Algorithms
    #include <cstring>      // [ ANSI C ] Strings
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      AUXILIARY FUNCTIONS
// =============================================================================


// normalize separators for better path output
string NormalizePath( const string& Path )
{
    string Result = Path;
    ReplaceCharacter( Result, '\\', '/' );
    ReplaceSubstring( Result, "//", "/" );
    
    // if needed remove ./ from the start
    if( Result[ 0 ] == '.' && Result[ 1 ] == '/' )
      Result = Result.substr( 2 );
    
    return Result;
}


// =============================================================================
//      FUNCTIONS TO OUTPUT FILES WITH DEBUG INFORMATION
// =============================================================================


void SaveDebugInfoFile( const string& FilePath, const VirconASMParser& Parser, const VirconASMEmitter& Emitter )
{
    if( VerboseMode )
      cout << "saving debug info file" << endl;

	int BaseOffset  = 0;
	int UnitSize    = 1;

	if( CartridgeOffset )
	{
		BaseOffset  = 0x8C;
		UnitSize    = 4;
	}
    
	if( VBINOffset )
	{
		BaseOffset  = 0x0C;
		UnitSize    = 4;
	}

    // open output file,
    ofstream DebugInfoFile;
    OpenOutputFile( DebugInfoFile, FilePath );
    
    if( DebugInfoFile.fail() )
      throw runtime_error( "cannot open debug info file \"" + FilePath + "\"" );
    
    // for each instruction in the file output a line with this
    // information (CSV format): ROM address, relative file path, line number
    for( ASTNode* Node: Parser.ProgramAST )
    {
        if( Node->Type() != ASTNodeTypes::Instruction )
          continue;
        

        DebugInfoFile << Hex( Node->AddressInROM * UnitSize + BaseOffset, 8 );
        DebugInfoFile << "," << NormalizePath( Node->Location.FilePath );
        DebugInfoFile << "," << Node->Location.Line;
        
        // check if this line corresponds to a label;
        // in that case add its name as a 4th column
        for( auto Pair : Emitter.LabelAddresses )
        {
            if( Pair.second == Node->AddressInROM )
            {
                DebugInfoFile << "," << Pair.first;
                break;
            }
        }
        
        DebugInfoFile << endl;
    }
    
    // close output
    DebugInfoFile.close();
}

// -----------------------------------------------------------------------------

void SaveLexerLog( const string& FilePath, const VirconASMPreprocessor& Preprocessor )
{
    if( VerboseMode )
      cout << "Debug mode: Saving lexer log" << endl;
    
    ofstream LogFile;
    OpenOutputFile( LogFile, FilePath );
    
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

void SaveParserLog( const string& FilePath, const VirconASMParser& Parser )
{
    if( VerboseMode )
      cout << "Debug mode: Saving parser log" << endl;
    
    ofstream LogFile;
    OpenOutputFile( LogFile, FilePath );
    
    if( LogFile.fail() )
      throw runtime_error( "cannot open parser log file \"" + FilePath + "\"" );
    
    // log full AST tree
    for( ASTNode* Node: Parser.ProgramAST )
      LogFile << Node->ToString() << endl;
    
    LogFile.close();
}

// -----------------------------------------------------------------------------

void SaveEmitterLog( const string& FilePath, const VirconASMParser& Parser )
{
    if( VerboseMode )
      cout << "Debug mode: Saving emitter log" << endl;
    
    ofstream LogFile;
    OpenOutputFile( LogFile, FilePath );
    
    if( LogFile.fail() )
      throw runtime_error( "cannot open emitter log file \"" + FilePath + "\"" );
    
    for( ASTNode* Node: Parser.ProgramAST )
      LogFile << Hex( Node->AddressInROM, 8 ) << ": " << Node->ToString() << endl;
    
    LogFile.close();
}
