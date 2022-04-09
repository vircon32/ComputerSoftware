// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconDataStructures.hpp"
    
    // include project headers
    #include "VirconASMLexer.hpp"
    
    // include C/C++ headers
    #include <cstring>      // [ ANSI C ] Stringds
    #include <stdexcept>    // [ C++ STL ] Exceptions
    #include <map>          // [ C++ STL ] Maps
    
    // declare used namespaces
    using namespace std;
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
    static const char OtherSeparators[] = ":,[]+-;";
    
    if( IsWhitespace( c ) ) return true;
    return( strchr( OtherSeparators, c ) != nullptr );
}


// =============================================================================
//      VIRCON ASM LEXER: INSTANCE HANDLING
// =============================================================================


VirconASMLexer::VirconASMLexer()
{
    Input = nullptr;
    CurrentReadLine = 1;
    PreviousChar = ' ';
}

// -----------------------------------------------------------------------------

VirconASMLexer::~VirconASMLexer()
{
    for( Token* T : Tokens )
      delete T;
}


// =============================================================================
//      VIRCON ASM LEXER: READING FROM INPUT
// =============================================================================


// use this instead of just Input->get()
// so that we can keep track of file position
char VirconASMLexer::GetChar()
{
    // don't attempt to get characters from an ended file
    if( Input->eof() ) return ' ';
    char c = Input->get();
    
    // always check that read characters are valid
    if( IsInvalidAscii( c ) )
      EmitError( "character is not valid (non-printable ASCII)" );
    
    // line break type 1: CR    
    if( c == '\r' )
      CurrentReadLine++;
    
    else if( c == '\n' )
    {
        // line break type 2: LF
        if( PreviousChar != '\r' )
          CurrentReadLine++;
        
        // line break type 3: CR + LF
        // (do nothing)
    }
    
    PreviousChar = c;
    return c;
}

// -----------------------------------------------------------------------------


// use this instead of just Input->peek()
// to reliably detect the end of file
char VirconASMLexer::PeekChar()
{
    int c = Input->peek();
    
    if( c == EOF )
      return GetChar();
    
    return (char)c;
}


// =============================================================================
//      VIRCON ASM LEXER: ERROR HANDLING
// =============================================================================


void VirconASMLexer::EmitError( const string& Description, bool Abort )
{
    cerr << "line " << CurrentReadLine << ": ";
    cerr << "lexer error: " << Description << endl;
    
    if( Abort )
      throw runtime_error( "assembly terminated" );
}

// -----------------------------------------------------------------------------

void VirconASMLexer::EmitWarning( const string& Description )
{
    cerr << "line " << CurrentReadLine << ": " << endl;
    cerr << "lexer warning: " << Description << endl;
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
        
        if( Input->eof() )
          return;
    }
}

// -----------------------------------------------------------------------------

void VirconASMLexer::SkipLineComment()
{
    int StartLine = CurrentReadLine;
    
    while( !Input->eof() )
    {
        GetChar();
        
        // detect comment end
        if( CurrentReadLine > StartLine )
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

LiteralIntegerToken* VirconASMLexer::ReadHexInteger()
{
    // first consume the "x" in prefix 0x
    GetChar();
    
    // accumulate the number as text
    string CurrentDigits;
    
    while( !Input->eof() )
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
    VirconWord NumberWord;
    
    try
    {
        NumberWord.AsBinary = stoul( CurrentDigits, nullptr, 16 );
        return NewIntegerToken( CurrentReadLine, NumberWord.AsInteger );
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
        
        if( !Input->eof() )
          if( (char)PeekChar() == 'x' )
            return ReadHexInteger();
    }
    
    // otherwise read as decimal
    while( !Input->eof() )
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
            return NewIntegerToken( CurrentReadLine, stoi( DigitsBeforeDot ) );
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
            return NewFloatToken( CurrentReadLine, stof( DigitsBeforeDot + '.' + DigitsAfterDot ) );
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
        
        if( Input->eof() )
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
    
    while( !Input->eof() )
    {
        char c = GetChar();
        
        // strings have to end in the same line
        if( c == '\r' || c == '\n' )
          EmitError( "string not terminated" );
        
        // detect escaped characters
        if( c == '\\' )
        {
            char c2 = GetChar();
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
//      VIRCON ASM LEXER: MAIN LEXER FUNCTION
// =============================================================================


void VirconASMLexer::ReadTokens( ifstream& Input_ )
{
    // reset any previous reads
    Input = &Input_;
    Input->clear();
    Input->seekg( 0, ios::beg );
    CurrentReadLine = 1;
    PreviousChar = ' ';
    
    // reset any previous results
    for( Token* T : Tokens )
      delete T;
    
    Tokens.clear();
    
    // start the list with an start-of-file indicator
    StartOfFileToken* FirstToken = new StartOfFileToken;
    FirstToken->LineInSource = CurrentReadLine;
    Tokens.push_back( FirstToken );
    
    // recognize all file text as tokens
    while( !Input->eof() )
    {
        char c = PeekChar();
        
        // whitespace is generally ignored, and serves only
        // as separator between tokens (required in some contexts)
        if( IsWhitespace( c ) )
        {
            SkipWhitespace();
            continue;
        }
        
        // character ';' is the start of a line comment
        if( c == ';' )
        {
            GetChar();
            SkipLineComment();
            continue;
        }
        
        // recognize the beginning of a string
        if( c == '\"' )
        {
            string StringValue = ReadString();
            Tokens.push_back( NewStringToken( CurrentReadLine, StringValue ) );
            continue;
        }
        
        // recognize punctuation characters
        if( c == ',' )
        {
            GetChar();
            Tokens.push_back( NewCommaToken( CurrentReadLine ) );
            continue;
        }
        
        if( c == ':' )
        {
            GetChar();
            Tokens.push_back( NewColonToken( CurrentReadLine ) );
            continue;
        }
        
        if( c == '[' )
        {
            GetChar();
            Tokens.push_back( NewOpenBracketToken( CurrentReadLine ) );
            continue;
        }
        
        if( c == ']' )
        {
            GetChar();
            Tokens.push_back( NewCloseBracketToken( CurrentReadLine ) );
            continue;
        }
        
        if( c == '+' )
        {
            GetChar();
            Tokens.push_back( NewPlusToken( CurrentReadLine ) );
            continue;
        }
        
        if( c == '-' )
        {
            GetChar();
            Tokens.push_back( NewMinusToken( CurrentReadLine ) );
            continue;
        }
        
        // anything starting with a digit will be taken as a number
        if( isdigit( c ) )
        {
            // this function will decide if it is an int, float of hex
            Tokens.push_back( ReadNumber() );
            continue;
        }
        
        // any other cases will be taken as idenfitiers
        string Name = ReadName();
        
        string NameUpper = Name;
        for( char& c : NameUpper ) c = toupper( c );
        
        // CASE 1: name is a hardware name
        if( IsRegisterName( Name ) )
          Tokens.push_back( NewRegisterToken( CurrentReadLine, StringToRegister(Name) ) );
        
        // CASE 2: name is an I/O port name
        else if( IsPortName( Name ) )
          Tokens.push_back( NewPortToken( CurrentReadLine, StringToPort(Name) ) );
        
        // CASE 3: name is an I/O port value name
        else if( IsPortValueName( Name ) )
          Tokens.push_back( NewPortValueToken( CurrentReadLine, StringToPortValue(Name) ) );
        
        // CASE 4: name is an instruction name
        else if( IsOpCodeName( Name ) )
          Tokens.push_back( NewOpCodeToken( CurrentReadLine, StringToOpCode(Name) ) );
        
        // CASE 5: name is an label idenfitier
        else if( Name[0] == '_' )
          Tokens.push_back( NewLabelToken( CurrentReadLine, Name ) );
        
        // CASE 6: name is the integer keyword
        else if( NameUpper == "INTEGER" )
          Tokens.push_back( NewIntegerKeywordToken( CurrentReadLine ) );
        
        // CASE 7: name is the float keyword
        else if( NameUpper == "FLOAT" )
          Tokens.push_back( NewFloatKeywordToken( CurrentReadLine ) );
        
        // CASE 8: name is the string keyword
        else if( NameUpper == "STRING" )
          Tokens.push_back( NewStringKeywordToken( CurrentReadLine ) );
        
        // CASE 9: name is the define keyword
        else if( NameUpper == "DEFINE" )
          Tokens.push_back( NewDefineKeywordToken( CurrentReadLine ) );
        
        // CASE 10: names is the datafile keyword
        else if( NameUpper == "DATAFILE" )
          Tokens.push_back( NewDataFileKeywordToken( CurrentReadLine ) );
        
        // CASE 11: other identifiers are taken as variable names
        else
          Tokens.push_back( NewVariableToken( CurrentReadLine, Name ) );
    }
    
    // complete the list with an end-of-file indicator
    EndOfFileToken* LastToken = new EndOfFileToken;
    LastToken->LineInSource = CurrentReadLine;
    Tokens.push_back( LastToken );
}
