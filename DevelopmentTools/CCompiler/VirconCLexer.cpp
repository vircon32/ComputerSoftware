// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "VirconCLexer.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <cstring>      // [ C++ STL ] Strings
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <map>          // [ C++ STL ] Maps
    
    // declare used namespaces
    using namespace std;
    using namespace V32;
// *****************************************************************************


// =============================================================================
//      CHARACTER CLASSIFICATION FUNCTIONS
// =============================================================================


bool IsAscii( char c )
{
    return !(c & 0x80);
}

// -----------------------------------------------------------------------------

bool IsWhitespace( char c )
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

// -----------------------------------------------------------------------------

bool IsPrintableAscii( char c )
{
    return (c >= 32 && !(c & 0x80));
}

// -----------------------------------------------------------------------------

bool IsInvalidAscii( char c )
{
    return (c >= 0) && (c < 32) && !IsWhitespace( c );
}

// -----------------------------------------------------------------------------

bool IsValidNameStart( char c )
{
    return IsPrintableAscii(c) && (isalpha(c) || (c == '_'));
}

// -----------------------------------------------------------------------------

bool IsValidNameContinuation( char c )
{
    return IsValidNameStart(c) || isdigit(c);
}

// -----------------------------------------------------------------------------

bool IsSeparator( char c )
{
    static const char Separators[] = "=+-*/!&|#~^%<>()[]{}:;,.";
    
    if( IsWhitespace( c ) ) return true;
    return( strchr( Separators, c ) != nullptr );
}


// =============================================================================
//      VIRCON C LEXER: INSTANCE HANDLING
// =============================================================================


VirconCLexer::VirconCLexer()
{
    ReadLocation.Line = 1;
    ReadLocation.LogicalLine = 1;
    ReadLocation.Column = 1;
    PreviousChar = ' ';
    LineIsContinued = false;
}

// -----------------------------------------------------------------------------

VirconCLexer::~VirconCLexer()
{
    for( CTokenList Line: TokenLines )
    {
        for( CToken* T : Line )
          delete T;
        
        Line.clear();
    }
    
    TokenLines.clear();
}


// =============================================================================
//      VIRCON C LEXER: READING FROM INPUT
// =============================================================================


// use this instead of just Input.get()
// so that we can keep track of file position
char VirconCLexer::GetChar()
{
    // don't attempt to get characters from an ended file
    char c = Input.get();
    if( Input.eof() ) return ' ';
    
    // always check that read characters are valid
    if( IsInvalidAscii( c ) )
      RaiseError( ReadLocation, "character is not valid (non-printable ASCII)" );
    
    // line break type 1: CR    
    if( c == '\r' )
    {
        ReadLocation.Line++;
        ReadLocation.Column = 1;
        
        if( LineIsContinued )
          LineIsContinued = false;
        else
          ReadLocation.LogicalLine = ReadLocation.Line;
    }
    
    else if( c == '\n' )
    {
        // line break type 2: LF
        if( PreviousChar != '\r' )
        {
            ReadLocation.Line++;
            ReadLocation.Column = 1;
            
            if( LineIsContinued )
              LineIsContinued = false;
            else
              ReadLocation.LogicalLine = ReadLocation.Line;
        }
        
        // line break type 3: CR + LF
        // (do not advance column)
    }
    
    // any other characters
    else ReadLocation.Column++;
    
    // save line continuations for later
    if( c == '\\' )
      LineIsContinued = true;
    else if( !IsWhitespace( c ) )
      LineIsContinued = false;
    
    PreviousChar = c;
    return c;
}

// -----------------------------------------------------------------------------

// use this instead of just Input.peek()
// to reliably detect the end of file
char VirconCLexer::PeekChar()
{
    int c = Input.peek();
    
    if( c == EOF )
      return GetChar();
    
    return (char)c;
}

// -----------------------------------------------------------------------------

// use this for reliability!
// (input will not set eof() until a read past the end has been attempted)
bool VirconCLexer::InputHasEnded()
{
    PeekChar();
    return Input.eof();
}


// =============================================================================
//      VIRCON C LEXER: SKIPPING FUNCTIONS
// =============================================================================


// another possible option is to not use peek,
// and instead use unget at the end (is it safe?)
void VirconCLexer::SkipWhitespace()
{
    while( IsWhitespace( (char)PeekChar() ) )
    {
        GetChar();
        
        if( InputHasEnded() )
          return;
    }
}

// -----------------------------------------------------------------------------

void VirconCLexer::SkipLineComment()
{
    SourceLocation StartLocation = ReadLocation;
    
    while( !InputHasEnded() )
    {
        char c = GetChar();
        
        // upon a stray, try to continue into next line
        // but only if it is the last character in the line!
        if( c == '\\' )
        {
            char Next = PeekChar();
            
            if( Next == '\n' || Next == '\r' )
              SkipLineAfterStray();
        }
        
        // detect comment end
        if( ReadLocation.Line > StartLocation.Line )
          return;
    }
}


// -----------------------------------------------------------------------------

void VirconCLexer::SkipBlockComment()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    char Previous = ' ';
    char Current = ' ';
    
    while( !InputHasEnded() )
    {
        Previous = Current;
        Current = GetChar();
        
        // detect comment end
        if( Previous == '*' && Current == '/')
          return;
    }
    
    RaiseFatalError( StartLocation, "block comment not terminated" );
}

// -----------------------------------------------------------------------------

void VirconCLexer::SkipLineAfterStray()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // there should not be whitespace
    bool ThereWasWhitespace = false;
    
    while( !InputHasEnded() )
    {
        char c = GetChar();
        
        if( ReadLocation.Line > StartLocation.Line )
          break;
        
        if( IsWhitespace( c ) )
          ThereWasWhitespace = true;
        
        else
          RaiseFatalError( ReadLocation, "characters found after backslash" );
    }
    
    // finally, warn if needed
    if( ThereWasWhitespace )
      RaiseWarning( StartLocation, "whitespace after backslash" );
}


// =============================================================================
//      VIRCON C LEXER: BASIC LEXER FUNCTIONS
// =============================================================================


char VirconCLexer::UnescapeCharacter( char Escaped )
{
    if( Escaped == 'n'  )  return '\n';
    if( Escaped == 'r'  )  return '\r';
    if( Escaped == 't'  )  return '\t';
    if( Escaped == '\\' )  return '\\';
    if( Escaped == '\'' )  return '\'';
    if( Escaped == '\"' )  return '\"';
    
    RaiseWarning( ReadLocation, string("unknown escape character '\\") + Escaped + "\'" );
    return Escaped;
}

// -----------------------------------------------------------------------------

char VirconCLexer::UnescapeHexNumber()
{
    // expect exactly 2 digits, and add a null string terminator
    char Digits[ 3 ] = { 0, 0, 0 };
    
    for( int i = 0; i < 2; i++ )
    {
        if( InputHasEnded() )
          RaiseFatalError( ReadLocation, "Unexpected end of file" );
        
        Digits[ i ] = GetChar();
        
        if( !isxdigit( Digits[ i ] ) )
        {
            RaiseError( ReadLocation, "bad hexadecimal character (expected 2 hex digits)" );
            return 0;
        }
    }
    
    // compose the hex character value
    char DecodedChar = (unsigned char)strtol( Digits, nullptr, 16 );
    return DecodedChar;
}

// -----------------------------------------------------------------------------

LiteralValueToken* VirconCLexer::ReadHexInteger()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // first consume the "x" in prefix 0x
    GetChar();
    
    // accumulate the number as text
    string CurrentDigits;
    
    while( !InputHasEnded() )
    {
        char c = PeekChar();
        
        if( isxdigit(c) )
        {
            CurrentDigits += c;
            GetChar();
        }
        
        else if( IsSeparator(c) )
          break;
        
        else
          RaiseFatalError( ReadLocation, "bad hexadecimal number literal" );
    }
    
    // convert to a number token
    // (use an unsigned, or else INT_MIN will throw an exception)
    V32Word NumberWord;
    
    try
    {
        NumberWord.AsBinary = stoul( CurrentDigits, nullptr, 16 );
        return NewIntToken( StartLocation, NumberWord.AsInteger );
    }
    catch( exception& e )
    {
        RaiseFatalError( ReadLocation, "number out of range for a 32-bit integer" );
    }
}

// -----------------------------------------------------------------------------

LiteralValueToken* VirconCLexer::ReadNumber()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // different parts of the number
    string DigitsBeforeDot;
    string DigitsAfterDot;
    string* CurrentDigits = &DigitsBeforeDot;
    
    // first, check for the hex notation prefix
    if( PeekChar() == '0' )
    {
        // needed for the case when
        // "0" is the whole number!!
        DigitsBeforeDot = "0";
        GetChar();
        
        if( !InputHasEnded() )
          if( (char)PeekChar() == 'x' )
            return ReadHexInteger();
    }
    
    // otherwise read as decimal
    while( !InputHasEnded() )
    {
        char c = PeekChar();
        
        if( isdigit(c) )
        {
            GetChar();
            (*CurrentDigits) += c;
        }
        
        else if( c == '.' && (CurrentDigits == &DigitsBeforeDot) )
        {
            GetChar();
            CurrentDigits = &DigitsAfterDot;
        }
        
        else if( IsSeparator(c) )
          break;
        
        else
          RaiseFatalError( ReadLocation, "bad floating point literal" );
    }
    
    // case 1: return an integer
    if( DigitsAfterDot.empty() )
    {
        try
        {
            return NewIntToken( StartLocation, stoi( DigitsBeforeDot ) );
        }
        catch( exception& e )
        {
            RaiseFatalError( ReadLocation, "number out of range for a 32-bit integer" );
        }
    }
    
    // case 2: return a float
    else
    {
        try
        {
            return NewFloatToken( StartLocation, stof( DigitsBeforeDot + '.' + DigitsAfterDot ) );
        }
        catch( exception& e )
        {
            RaiseFatalError( ReadLocation, "number out of range for a float" );
        }
    }
}

// -----------------------------------------------------------------------------

// characters are read as ints, but limiting
// their value to range [0-255]
LiteralValueToken* VirconCLexer::ReadCharacter()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // first, consume the start delimiter
    GetChar();
    
    int32_t Value = 0;
    char c = GetChar();
    
    // process escaped characters
    if( c == '\\' )
    {
        char c2 = GetChar();
        
        if( c2 == 'x' )
          Value = (unsigned char)UnescapeHexNumber();
        else
          Value = (unsigned char)UnescapeCharacter( c2 );
    }
    
    // process regular characters
    else Value = (unsigned char)c;
        
    // expect the closing delimiter
    if( InputHasEnded() || (GetChar() != '\'') )
      RaiseFatalError( StartLocation, "character not terminated" );
    
    return NewIntToken( StartLocation, Value );
}

// -----------------------------------------------------------------------------

LiteralStringToken* VirconCLexer::ReadString()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // first, consume the start delimiter
    GetChar();
    string Value = "";
    
    while( !InputHasEnded() )
    {
        char c = GetChar();
        
        // strings have to end in the same line
        if( ReadLocation.LogicalLine != StartLocation.LogicalLine )
          RaiseFatalError( StartLocation, "string not terminated" );
        
        // detect escaped characters
        if( c == '\\' )
        {
            char c2 = GetChar();
            
            if( c2 == 'x' )
              Value += (unsigned char)UnescapeHexNumber();
            else
              Value += (unsigned char)UnescapeCharacter( c2 );
            
            continue;
        }
        
        // detect string end
        if( c == '\"' )
          return NewStringToken( StartLocation, Value );
        
        // any other character is just added
        Value += c;
    }
    
    RaiseFatalError( StartLocation, "string not terminated" );
    
    // avoid compiler warning
    return nullptr;
}

// -----------------------------------------------------------------------------

OperatorToken* VirconCLexer::ReadOperator()
{
    // capture start location
    SourceLocation StartLocation = ReadLocation;
    
    // start with the first character
    string Name;
    Name += GetChar();
    
    // now keep reading as long as the operator is still valid
    while( !InputHasEnded() )
    {
        string ExtendedName = Name + PeekChar();
        
        if( IsOperator( ExtendedName ) )
          Name += GetChar();
        else
          break;
    }
    
    return NewOperatorToken( StartLocation, WhichOperator(Name) );
}

// -----------------------------------------------------------------------------

string VirconCLexer::ReadName()
{
    // the first character follows different rules
    string Name;
    char c = GetChar();
    
    if( !IsValidNameStart( c ) )
      RaiseFatalError( ReadLocation, string("character '") + c + "' is not a valid identifier start" );
    
    Name = c;
    
    // keep adding to the name
    while( true )
    {
        c = PeekChar();
        
        if( InputHasEnded() )
          break;
        
        if( IsValidNameContinuation( c ) )
        {
            Name += c;
            GetChar();
        }
        
        // stop at all valid separators
        else if( IsSeparator( c ) )
          break;
        
        // otherwise, it is an error
        else
          RaiseFatalError( ReadLocation, string("character '") + c + "' is not valid in an identifier" );
    }
    
    return Name;
}


// =============================================================================
//      VIRCON C LEXER: MAIN LEXER FUNCTIONS
// =============================================================================


void VirconCLexer::SkipUntilNextToken()
{
    if( InputHasEnded() )
      return;
    
    char c = PeekChar();
    
    // whitespace is generally ignored, and serves only
    // as separator between tokens (required in some contexts)
    if( IsWhitespace( c ) )
    {
        SkipWhitespace();
        SkipUntilNextToken();
        return;
    }
    
    // process line continuation at the lexer level
    if( c == '\\' )
    {
        GetChar();
        SkipLineAfterStray();
        SkipUntilNextToken();
        return;
    }
    
    // character '/' may be the start of comments
    if( c == '/' )
    {
        SourceLocation LastLocation = ReadLocation;
        char LastPreviousChar = PreviousChar;
        
        GetChar();
        char c2 = PeekChar();
        
        if( c2 == '/' )
        {
            GetChar();
            SkipLineComment();
            SkipUntilNextToken();
            return;
        }
        
        if( c2 == '*' )
        {
            GetChar();
            SkipBlockComment();
            SkipUntilNextToken();
            return;
        }
        
        // otherwise, go back 1 character
        // and let it be processed as an operator
        Input.unget();
        ReadLocation = LastLocation;
        PreviousChar = LastPreviousChar;
        return;
    }
}

// -----------------------------------------------------------------------------

// Note that no keywords are recognized here!!
// Until preprocessing is done, they are taken as identifiers
CToken* VirconCLexer::ReadNextToken()
{
    // peek the first useful character
    SkipUntilNextToken();
    char c = PeekChar();
    
    // capture its start location
    SourceLocation StartLocation = ReadLocation;
    
    // first: recognize special symbols
    if( IsSpecialSymbol( c ) )
    {
        GetChar();
        return NewSpecialSymbolToken( StartLocation, WhichSpecialSymbol(c) );
    }
    
    // second: recognize delimiters
    if( IsDelimiter( c ) )
    {
        GetChar();
        return NewDelimiterToken( StartLocation, WhichDelimiter(c) );
    }
    
    // operators are recognized greedily (longest: "-->" is "--",">")
    // (also, after skips, character '/' may only be an operator)
    if( IsOperator( string(1,c) ) )
      return ReadOperator();
    
    // anything starting with a digit will be taken as a number
    if( isdigit( c ) )
    {
        // may be either int or float
        // (NOTE 1: for ints, we are not yet supporting octals/binary/hexadecimal)
        // (NOTE 2: for floats, we are not yet supporting scientific notation)
        return ReadNumber();
    }
    
    // start of character
    if( c == '\'' )
      return ReadCharacter();
    
    // start of string
    if( c == '\"' )
      return ReadString();
    
    // any other cases will be taken as idenfitiers
    string Name = ReadName();
    
    // CASE 1: name is a boolean literal
    if( Name == "true" || Name == "false" )
      return NewBoolToken( StartLocation, (Name == "true") );
    
    // CASE 2: name is an actual idenfitier
    else
      return NewIdentifierToken( StartLocation, Name );
}

// -----------------------------------------------------------------------------

void VirconCLexer::OpenFile( const string& FilePath )
{
    // reset any previous state
    Input.close();
    Input.clear();
    
    // open the file as binary, not as text!
    // (otherwise there can be bugs using tellg/seekg and unget)
    Input.open( FilePath, ios_base::in | ios_base::binary );
    Input.seekg( 0, ios::beg );
    
    if( Input.fail() )
      throw runtime_error( "cannot open input file \"" + FilePath + "\"" );
    
    // capture the input file directory
    InputDirectory = GetPathDirectory( FilePath );
    
    // reset any previous reads
    ReadLocation.FilePath = FilePath;
    ReadLocation.Line = 1;
    ReadLocation.LogicalLine = 1;
    ReadLocation.Column = 1;
    PreviousChar = ' ';
    LineIsContinued = false;
    
    // reset any previous results
    for( CTokenList Line: TokenLines )
    {
        for( CToken* T : Line )
          delete T;
        
        Line.clear();
    }
    
    TokenLines.clear();
    
    // start the list with an start-of-file indicator
    StartOfFileToken* FirstToken = new StartOfFileToken;
    FirstToken->Location = ReadLocation;
    
    TokenLines.emplace_back();
    TokenLines.back().push_back( FirstToken );
}

// -----------------------------------------------------------------------------

void VirconCLexer::CloseFile()
{
    // we are finished with the input file
    Input.close();
    
    // complete the list with an end-of-file indicator
    EndOfFileToken* LastToken = new EndOfFileToken;
    LastToken->Location = ReadLocation;
    
    TokenLines.emplace_back();
    TokenLines.back().push_back( LastToken );
}

// -----------------------------------------------------------------------------

CTokenList VirconCLexer::TokenizeNextLine()
{
    // first, find the actual start of the line
    SkipUntilNextToken();
    
    // then keep reading until a new logical line is found
    int InitialLine = ReadLocation.LogicalLine;
    CTokenList TokenLine;
    
    while( !InputHasEnded() && ReadLocation.LogicalLine == InitialLine )
    {
        TokenLine.push_back( ReadNextToken() );
        SkipUntilNextToken();
    }
    
    return TokenLine;
}

// -----------------------------------------------------------------------------

void VirconCLexer::TokenizeFile( const std::string& FilePath )
{
    OpenFile( FilePath );
    
    // recognize all file text as tokens
    while( !InputHasEnded() )
      TokenLines.push_back( TokenizeNextLine() );
    
    CloseFile();
}
