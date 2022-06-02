// *****************************************************************************
    // include project headers
    #include "LogStream.hpp"
    
    // include C/C++ headers
    #include <iostream>             // [ C++ STL ] I/O Streams
    #include <iomanip>              // [ C++ STL ] I/O Manipulation
    #include <time.h>
    
    // include SDL2 headers
    #include <SDL2/SDL_rwops.h>     // [ SDL2 ] R/W Operations
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//		LOG STREAM: INSTANCE HANDLING
// =============================================================================


LogStream::LogStream()
// - - - - - - - - - - - -
:   IndentPending( true )
,   OpenBracePending( false )
,   ConsoleMode( true )
,   Indent( 0 )
// - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

LogStream::~LogStream()
{
    CloseFile();
}

// =============================================================================
//      LOG STREAM: FILE HANDLING
// =============================================================================


bool LogStream::OpenFile( const string& File, const string& Title )
{
    // close previous file, if any
    CloseFile();
    OutputFile.clear();
    
    // attempt to open the file
    OutputFile.open( File.c_str(), ios::out );
    
    // if file didn't open, report and use the console
    if( !OutputFile.is_open() || OutputFile.bad() )
    {
        ConsoleMode = true;
        
        operator<<( "ERROR: Could not open log file \"" );
        operator<<( File );
        operator<<( "\". Logging to console." );
        AddLine();
        
        return false;
    }
    
    // configure log to use the file
    ConsoleMode = false;
    
    // disable any indent
    Indent = 0;
    
    // write an UTF-8 BOM to enable unicode output
    // (but keeping normal ASCII as valid)
    unsigned char UTF8BOM[] = { 0xEF, 0xBB, 0xBF };
    OutputFile.write( (char*)UTF8BOM, 3 );
    
    // write the title at the beginning
    OutputFile.seekp( 0, ios::beg );
    operator<<( "Log Title: \"" );
    operator<<( Title );
    operator<<( "\", started on: " );
    
    // write date and time
    WriteDate();
    WriteTime();
    AddLine();
    
    // write separator
    operator<<( "--------------------------------------------------------------------------------" );
    AddLine();
    
    // configure
    OutputFile << setprecision(8);
    
    return true;
}

// -----------------------------------------------------------------------------

void LogStream::CloseFile()
{
    if( !OutputFile.is_open() || OutputFile.bad() )
    return;
    
    // configure log to use the file
    ConsoleMode = false;
    
    // disable any indent
    Indent = 0;
    
    // write separator
    operator<<( "--------------------------------------------------------------------------------" );
    AddLine();
    
    // write date and time at the end
    OutputFile.seekp( 0, ios::end );
    operator<<( "Log finished on: " );
    WriteDate();
    WriteTime();
    AddLine();
    
    // close file
    OutputFile.close();
    
    // now there is no file, so use the console
    ConsoleMode = true;
}


// =============================================================================
//		LOG STREAM: SCOPE HANDLING
// =============================================================================


void LogStream::OpenScope()
{
    OpenBracePending = true;
}

// -----------------------------------------------------------------------------

void LogStream::CloseScope()
{
    if( !OpenBracePending )
    {
        Indent--;
        LOG( '}' ); 
    }
    
    else OpenBracePending = false;
}


// =============================================================================
//		LOG STREAM: AUXILIARY METHODS
// =============================================================================


ostream& LogStream::Target()
{
    return ConsoleMode? cout : OutputFile;
}

// -----------------------------------------------------------------------------

void LogStream::AddLine()
{
    if( !Target().bad() )
    Target() << EndOfLine;
    
    IndentPending = true;
}

// -----------------------------------------------------------------------------

void LogStream::WriteIndent()
{
    if( !IndentPending )
    return;
    
    if( OpenBracePending )
    {
        OpenBracePending = false;
        
        LOG( '{' );
        //operator<< message << EndOfLine
        //Flush();
        
        Indent++;
    }
    
    if( !Target().bad() )
      for( int i = 0; i < Indent; i++ )
        Target() << "    ";
    
    IndentPending = false;
}

// -----------------------------------------------------------------------------

void LogStream::WriteDate()
{
    time_t CurrentTime;
    struct tm* TimeInfo;
    char Buffer[ 80 ];

    // validate target stream
    if( Target().bad() )
    return;
    
    // write indent when needed
    WriteIndent();
    
    // obtain current time
    time( &CurrentTime );
    TimeInfo = localtime( &CurrentTime );
    
    // build string and write it to file
    strftime( Buffer, 80, "[%A, %B %d, %Y]",TimeInfo );
    Target() << Buffer;
}

// -----------------------------------------------------------------------------

void LogStream::WriteTime()
{
    time_t CurrentTime;
    struct tm* TimeInfo;
    char Buffer[ 30 ];

    // validate target stream
    if( Target().bad() )
    return;

    // write indent when needed
    WriteIndent();
    
    // obtain current time
    time( &CurrentTime );
    TimeInfo = localtime( &CurrentTime );
    
    // build string and write it to file
    strftime( Buffer, 30, "[%H:%M:%S]",TimeInfo );
    Target() << Buffer;
}

// -----------------------------------------------------------------------------

void LogStream::Flush()
{
    if( !Target().bad() )
    Target().flush();
}


// =============================================================================
//		LOG STREAM: FORMATTED OUTPUT
// =============================================================================


LogStream& LogStream::operator<<( const string& Message )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    if( !Message.empty() )
    Target() << Message;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( const char* Message )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    if( Message && (*Message) )
    Target() << Message;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( void* Pointer )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Pointer;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( bool Condition )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << (Condition? "true" : "false" );
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( char Character )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    {
        if( Character != EndOfLine )
        Target() << Character;
        
        else AddLine();
    }
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( short Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( int Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( long Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( unsigned char Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << (unsigned)Number;  // print the number, not the character
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( unsigned short Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( unsigned int Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( unsigned long Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( float Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}

// -----------------------------------------------------------------------------

LogStream& LogStream::operator<<( double Number )
{
    // write indent when needed
    WriteIndent();
    
    if( !Target().bad() )
    Target() << Number;
    
    return *this;
}


// =============================================================================
//		LOG STREAM: UNFORMATTED OUTPUT
// =============================================================================


void LogStream::WriteByte( unsigned char Data )
{
    // write indent when needed
    WriteIndent();
    
    OutputFile.put( Data );
}

// -----------------------------------------------------------------------------

void LogStream::WriteBuffer( const void* Buffer, unsigned long Bytes )
{
    // write indent when needed
    WriteIndent();
    
    OutputFile.write( (char*)Buffer, Bytes );
}


// =============================================================================
//		LOG STREAM: ADDING ENTRIES
// =============================================================================


// this function automates the usual log format
void LogStream::NormalEntry( const string& Entry )
{		
    operator<<( Entry );
    AddLine();
    Flush();
}

// -----------------------------------------------------------------------------

// this function automates the usual log format
void LogStream::TimedEntry( const string& Entry )
{		
    WriteTime();
    operator<<( Entry );
    AddLine();
    Flush();
}


// =============================================================================
//     WRAPPER FUNCTIONS FOR THE GLOBAL LOG
// =============================================================================


// provide external access to a single local instance
LogStream& GlobalLog()
{
    // instance created on first call
    static LogStream GlobalLogStream;
    
    // provide reference to instance
    return GlobalLogStream;
}

// -----------------------------------------------------------------------------

// wrapper function start the global log to a file
void LOG_TO_FILE( const string& Name )
{
    string FileName = Name + ".txt";
    string Title = "Process log for ";
    Title += Name;
    
    GlobalLog().OpenFile( FileName, Title );
}

// -----------------------------------------------------------------------------

// wrapper function start the global log to the console
void LOG_TO_CONSOLE()
{
    // close any previous files
    GlobalLog().CloseFile();
    
    // enable console
    GlobalLog().ConsoleMode = true;
}

// -----------------------------------------------------------------------------

// wrapper function to close the global log
void LOG_END()
{
    GlobalLog().CloseFile();
}


// =============================================================================
//		LOG SCOPE: CLASS IMPLEMENTATION
// =============================================================================


// only for the end of this file, redefine the global log to write logs with macros
#define GlobalLog() TargetLog

// -----------------------------------------------------------------------------

LogScope::LogScope( const string& Title, LogStream& Log )
// - - - - - - - - - - - - - - - - - -
:   TargetLog( Log )
// - - - - - - - - - - - - - - - - - -
{
    LOG( Title );
    TargetLog.OpenScope();
}

// -----------------------------------------------------------------------------

LogScope::~LogScope()
{
    TargetLog.CloseScope();
}
