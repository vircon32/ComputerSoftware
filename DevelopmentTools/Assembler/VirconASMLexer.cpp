// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/DataStructures.hpp"
    
    // include infrastructure headers
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    #include "../DevToolsInfrastructure/FilePaths.hpp"
    
    // include project headers
    #include "VirconASMLexer.hpp"
    
    // include C/C++ headers
    #include <cstring>      // [ ANSI C ] Stringds
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

bool IsPrintableAscii( char c )
{
    // faster version of: (c >= 32 && c < 128)
    return ( (c & 0b11100000) && !(c & 0b10000000) );
}

// -----------------------------------------------------------------------------

bool IsInvalidAscii( char c )
{
    return (c >= 0) && (c < 32) && !IsWhitespace( c );
}

// -----------------------------------------------------------------------------

bool IsWhitespace( char c )
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
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
    static const char OtherSeparators[] = ":,[]+-%;";
    
    if( IsWhitespace( c ) ) return true;
    return( strchr( OtherSeparators, c ) != nullptr );
}


// =============================================================================
//      VIRCON ASM LEXER: INSTANCE HANDLING
// =============================================================================


VirconASMLexer::VirconASMLexer()
{
    ReadLocation.Line = 1;
    PreviousChar = ' ';
}

// -----------------------------------------------------------------------------

VirconASMLexer::~VirconASMLexer()
{
    for( TokenList Line: TokenLines )
    {
        for( Token* T : Line )
          delete T;
        
        Line.clear();
    }
    
    TokenLines.clear();
}


// =============================================================================
//      VIRCON ASM LEXER: INPUT FILE HANDLING
// =============================================================================


void VirconASMLexer::OpenFile( const string& FilePath )
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
    PreviousChar = ' ';
    
    // reset any previous results
    for( TokenList Line: TokenLines )
    {
        for( Token* T : Line )
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

void VirconASMLexer::CloseFile()
{
    // we are finished with the input file
    Input.close();
    
    // complete the list with an end-of-file indicator
    EndOfFileToken* LastToken = new EndOfFileToken;
    LastToken->Location = ReadLocation;
    
    TokenLines.emplace_back();
    TokenLines.back().push_back( LastToken );
}


// =============================================================================
//      VIRCON ASM LEXER: READING FROM INPUT
// =============================================================================


// use this instead of just Input.get()
// so that we can keep track of file position
char VirconASMLexer::GetChar()
{
    // don't attempt to get characters from an ended file
    if( Input.eof() ) return ' ';
    char c = Input.get();
    
    // always check that read characters are valid
    if( IsInvalidAscii( c ) )
      EmitError( "character is not valid (non-printable ASCII)" );
    
    // line break type 1: CR    
    if( c == '\r' )
      ReadLocation.Line++;
    
    else if( c == '\n' )
    {
        // line break type 2: LF
        if( PreviousChar != '\r' )
          ReadLocation.Line++;
        
        // line break type 3: CR + LF
        // (do nothing)
    }
    
    PreviousChar = c;
    return c;
}

// -----------------------------------------------------------------------------

// use this instead of just Input.peek()
// to reliably detect the end of file
char VirconASMLexer::PeekChar()
{
    int c = Input.peek();
    
    if( c == EOF )
      return GetChar();
    
    return (char)c;
}

// -----------------------------------------------------------------------------

// use this for reliability!
// (input will not set eof() until a read past the end has been attempted)
bool VirconASMLexer::InputHasEnded()
{
    PeekChar();
    return Input.eof();
}


// =============================================================================
//      VIRCON ASM LEXER: ERROR HANDLING
// =============================================================================


void VirconASMLexer::EmitError( const string& Description, bool Abort )
{
    cerr << ReadLocation.FilePath << ':' << ReadLocation.Line;
    cerr << ": lexer error: " << Description << endl;
    
    if( Abort )
      throw runtime_error( "assembly terminated" );
}

// -----------------------------------------------------------------------------

void VirconASMLexer::EmitWarning( const string& Description )
{
    cerr << ReadLocation.FilePath << ':' << ReadLocation.Line;
    cerr << ": lexer warning: " << Description << endl;
}


// =============================================================================
//      VIRCON ASM LEXER: SKIPPING FUNCTIONS
// =============================================================================


// another possible option is to not use peek,
// and instead use unget at the end (is it safe?)
void VirconASMLexer::SkipWhitespace()
{
    while( IsWhitespace( (char)PeekChar() ) )
    {
        GetChar();
        
        if( InputHasEnded() )
          return;
    }
}

// -----------------------------------------------------------------------------

void VirconASMLexer::SkipLineComment()
{
    int StartLine = ReadLocation.Line;
    
    while( !InputHasEnded() )
    {
        GetChar();
        
        // detect comment end
        if( ReadLocation.Line > StartLine )
          return;
    }
}


// =============================================================================
//      VIRCON ASM LEXER: BASIC LEXER FUNCTIONS
// =============================================================================


char VirconASMLexer::UnescapeCharacter( char Escaped )
{
    if( Escaped == 'n'  )  return '\n';
    if( Escaped == 'r'  )  return '\r';
    if( Escaped == 't'  )  return '\t';
    if( Escaped == '\\' )  return '\\';
    if( Escaped == '\'' )  return '\'';
    if( Escaped == '\"' )  return '\"';
    
    EmitWarning( string("unknown escape character '\\") + Escaped + "\'" );
    return Escaped;
}

// -----------------------------------------------------------------------------

char VirconASMLexer::UnescapeHexNumber()
{
    // expect exactly 2 digits, and add a null string terminator
    char Digits[ 3 ] = { 0, 0, 0 };
    
    for( int i = 0; i < 2; i++ )
    {
        if( InputHasEnded() )
        {
            EmitError( "Unexpected end of file" );
            throw runtime_error( "Aborted" );
        }
        
        Digits[ i ] = GetChar();
        
        if( !isxdigit( Digits[ i ] ) )
        {
            EmitError( "bad hexadecimal character (expected 2 hex digits)" );
            return 0;
        }
    }
    
    // compose the hex character value
    char DecodedChar = (unsigned char)strtol( Digits, nullptr, 16 );
    return DecodedChar;
}

// -----------------------------------------------------------------------------

char VirconASMLexer::ReadCharacter()
{
    // first, consume the start delimiter
    GetChar();
    
    char Value = 0;
    char c = GetChar();
    
    // process escaped characters
    if( c == '\\' )
    {
        char c2 = GetChar();
        
        if( c2 == 'x' )
          Value = UnescapeHexNumber();
        else
          Value = UnescapeCharacter( c2 );
    }
    
    // process regular characters
    else Value = c;
        
    // expect the closing delimiter
    if( InputHasEnded() || (GetChar() != '\'') )
      EmitError( "character not terminated" );
    
    return Value;
}

// -----------------------------------------------------------------------------

LiteralIntegerToken* VirconASMLexer::ReadHexInteger()
{
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
          EmitError( "bad hexadecimal number literal" );
    }
    
    // convert to a number token
    // (use an unsigned, or else INT_MIN will throw an exception)
    V32Word NumberWord;
    
    try
    {
        NumberWord.AsBinary = stoul( CurrentDigits, nullptr, 16 );
        return NewIntegerToken( ReadLocation, NumberWord.AsInteger );
    }
    catch( exception& e )
    {
        EmitError( "number out of range for a 32-bit integer" );
        throw runtime_error( "Aborted" );
    }
}

// -----------------------------------------------------------------------------

Token* VirconASMLexer::ReadNumber()
{
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
        {
            EmitError( "bad floating point literal" );
            throw runtime_error( "Aborted" );
        }
    }
    
    // case 1: return an integer
    if( DigitsAfterDot.empty() )
    {
        try
        {
            return NewIntegerToken( ReadLocation, stoi( DigitsBeforeDot ) );
        }
        catch( exception& e )
        {
            EmitError( "number out of range for a 32-bit integer" );
            throw runtime_error( "Aborted" );
        }
    }
    
    // case 2: return a float
    else
    {
        try
        {
            return NewFloatToken( ReadLocation, stof( DigitsBeforeDot + '.' + DigitsAfterDot ) );
        }
        catch( exception& e )
        {
            EmitError( "number out of range for a float" );
            throw runtime_error( "Aborted" );
        }
    }
}

// -----------------------------------------------------------------------------

string VirconASMLexer::ReadName()
{
    string Name;
    char c = GetChar();
    
    if( !IsValidNameStart( c ) )
      EmitError( string("character '") + c + "' is not a valid identifier start" );
    
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
          EmitError( string("character '") + c + "' is not valid in an identifier" );
    }
    
    return Name;
}

// -----------------------------------------------------------------------------

string VirconASMLexer::ReadString()
{
    string Value = "";
    
    // first, consume the start delimiter
    GetChar();
    
    while( !InputHasEnded() )
    {
        char c = GetChar();
        
        // strings have to end in the same line
        if( c == '\r' || c == '\n' )
          EmitError( "string not terminated" );
        
        // detect escaped characters
        if( c == '\\' )
        {
            char c2 = GetChar();
            
            if( c2 == 'x' )
              Value += UnescapeHexNumber();
            else
              Value += UnescapeCharacter( c2 );
            
            continue;
        }
        
        // detect string end
        if( c == '\"' )
          return Value;
        
        // any other character is just added
        Value += c;
    }
    
    EmitError( "string not terminated" );
    
    // avoid compiler warning
    return "";
}


// =============================================================================
//      VIRCON ASM LEXER: MAIN LEXER FUNCTIONS
// =============================================================================


void VirconASMLexer::SkipUntilNextToken()
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
    
    // character ';' will be the start of comments
    if( c == ';' )
    {
        GetChar();
        SkipLineComment();
        SkipUntilNextToken();
        return;
    }
}

// -----------------------------------------------------------------------------

Token* VirconASMLexer::ReadNextToken()
{
    // peek the first useful character
    SkipUntilNextToken();
    char c = PeekChar();
    
    // recognize the beginning of a string
    if( c == '\"' )
      return NewStringToken( ReadLocation, ReadString() );
    
    // recognize the beginning of a character
    if( c == '\'' )
    {
        // prevent unwanted sign extension if char
        // is taken as negative (byte value > 127)
        union
        {
            char AsNumber;
            unsigned char AsBinary;
        }
        Character;
        
        Character.AsNumber = ReadCharacter();
        return NewIntegerToken( ReadLocation, Character.AsBinary );
    }
    
    // recognize symbols
    char SymbolString[ 2 ] = { c, 0 };
    
    if( IsSymbol( SymbolString ) )
    {
        GetChar();
        return NewSymbolToken( ReadLocation, WhichSymbol( SymbolString ) );
    }
    
    // anything starting with a digit will be taken as a number
    if( isdigit( c ) )
      return ReadNumber();
    
    // any other cases will be taken as idenfitiers
    string Name = ReadName();
    
    // ASM matches identifies regardless of case,
    // so for comparisons unify them into uppercase
    string NameUpper = Name;
    for( char& c : NameUpper ) c = toupper( c );
    
    // CASE 1: name is a label idenfitier
    if( Name[ 0 ] == '_' )
      return NewLabelToken( ReadLocation, Name );
    
    // CASE 2: name is a boolean value
    if( NameUpper == "TRUE" )
      return NewIntegerToken( ReadLocation, 1 );
    if( NameUpper == "FALSE" )
      return  NewIntegerToken( ReadLocation, 0 );
    
    // CASE 3: name is a hardware name
    if( IsRegisterName( Name ) )
      return NewRegisterToken( ReadLocation, StringToRegister(Name) );
    
    // CASE 4: name is an I/O port name
    if( IsPortName( Name ) )
      return NewPortToken( ReadLocation, StringToPort(Name) );
    
    // CASE 5: name is an I/O port value name
    if( IsPortValueName( Name ) )
      return NewPortValueToken( ReadLocation, StringToPortValue(Name) );
    
    // CASE 6: name is an instruction name
    if( IsOpCodeName( Name ) )
      return NewOpCodeToken( ReadLocation, StringToOpCode(Name) );
    
    // CASE 7: name is a keyword
    if( IsKeyword( Name ) )
      return NewKeywordToken( ReadLocation, WhichKeyword(Name) );
    
    // CASE 8: other names are taken as just plain identifiers
    return NewIdentifierToken( ReadLocation, Name );
}

// -----------------------------------------------------------------------------

TokenList VirconASMLexer::TokenizeNextLine()
{
    // first, find the actual start of the line
    SkipUntilNextToken();
    
    // then keep reading until a new line is found
    int InitialLine = ReadLocation.Line;
    TokenList TokenLine;
    
    while( !InputHasEnded() && ReadLocation.Line == InitialLine )
    {
        TokenLine.push_back( ReadNextToken() );
        SkipUntilNextToken();
    }
    
    return TokenLine;
}

// -----------------------------------------------------------------------------

void VirconASMLexer::TokenizeFile( const std::string& FilePath )
{
    OpenFile( FilePath );
    
    // recognize all file text as tokens
    while( !InputHasEnded() )
      TokenLines.push_back( TokenizeNextLine() );
    
    CloseFile();
}
