// *****************************************************************************
    // include project headers
    #include "Logger.hpp"
    
    // include C/C++ headers
    #include <iostream>         // [ C++ STL ] I/O Streams
    #include <iomanip>          // [ C++ STL ] I/O Manipulation
    #include <stdexcept>        // [ C++ STL ] Exceptions
    #include <time.h>           // [ ANSI C ] Time
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//		LOGGER: INSTANCE HANDLING
// =============================================================================


Logger::Logger()
// - - - - - - - - - - - -
:   ConsoleMode( true )
// - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

Logger::~Logger()
{
    CloseFile();
}

// =============================================================================
//      LOGGER: FILE HANDLING
// =============================================================================


bool Logger::OpenFile( const string& File, const string& Title )
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
        
        WriteString( "ERROR: Could not open log file \"" );
        WriteString( File );
        WriteString( "\". Logging to console." );
        AddLine();
        
        return false;
    }
    
    // configure log to use the file
    ConsoleMode = false;
    
    // write an UTF-8 BOM to enable unicode output
    // (but keeping normal ASCII as valid)
    unsigned char UTF8BOM[] = { 0xEF, 0xBB, 0xBF };
    OutputFile.write( (char*)UTF8BOM, 3 );
    
    // write the title at the beginning
    OutputFile.seekp( 0, ios::beg );
    WriteString( "Log title: \"" );
    WriteString( Title );
    WriteString( "\", started on: " );
    
    // write date and time
    WriteDate();
    WriteTime();
    AddLine();
    
    // write separator
    WriteString( "--------------------------------------------------------------------------------" );
    AddLine();
    
    // configure
    OutputFile << setprecision(8);
    
    return true;
}

// -----------------------------------------------------------------------------

void Logger::CloseFile()
{
    if( !OutputFile.is_open() || OutputFile.bad() )
      return;
    
    // configure log to use the file
    ConsoleMode = false;
    
    // write separator
    WriteString( "--------------------------------------------------------------------------------" );
    AddLine();
    
    // write date and time at the end
    OutputFile.seekp( 0, ios::end );
    WriteString( "Log finished on: " );
    WriteDate();
    WriteTime();
    AddLine();
    
    // close file
    OutputFile.close();
    
    // now there is no file, so use the console
    ConsoleMode = true;
}


// =============================================================================
//		LOGGER: LOG METHODS
// =============================================================================


ostream& Logger::Target()
{
    return ConsoleMode? cout : OutputFile;
}

// -----------------------------------------------------------------------------

void Logger::AddLine()
{
    if( !Target().bad() )
      Target() << endl;
}

// -----------------------------------------------------------------------------

void Logger::WriteDate()
{
    time_t CurrentTime;
    struct tm* TimeInfo;
    char Buffer[ 80 ];

    // validate target stream
    if( Target().bad() )
      return;
    
    // obtain current time
    time( &CurrentTime );
    TimeInfo = localtime( &CurrentTime );
    
    // build string and write it to file
    strftime( Buffer, 80, "[%A, %B %d, %Y]",TimeInfo );
    Target() << Buffer;
}

// -----------------------------------------------------------------------------

void Logger::WriteTime()
{
    time_t CurrentTime;
    struct tm* TimeInfo;
    char Buffer[ 30 ];

    // validate target stream
    if( Target().bad() )
      return;

    // obtain current time
    time( &CurrentTime );
    TimeInfo = localtime( &CurrentTime );
    
    // build string and write it to file
    strftime( Buffer, 30, "[%H:%M:%S]",TimeInfo );
    Target() << Buffer;
}

// -----------------------------------------------------------------------------

void Logger::Flush()
{
    if( !Target().bad() )
      Target().flush();
}

// -----------------------------------------------------------------------------

void Logger::WriteString( const string& Message )
{
    if( !Target().bad() )
      if( !Message.empty() )
        Target() << Message;
}


// =============================================================================
//     WRAPPER FUNCTIONS FOR THE GLOBAL LOG
// =============================================================================


// provide external access to a single local instance
Logger& GlobalLog()
{
    // instance created on first call
    static Logger GlobalLogger;
    
    // provide reference to instance
    return GlobalLogger;
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

// -----------------------------------------------------------------------------

void LOG( const std::string& Message )
{
    GlobalLog().WriteString( Message );
    GlobalLog().AddLine();
    GlobalLog().Flush();
}

// -----------------------------------------------------------------------------

[[ noreturn ]] void THROW( const std::string& Message )
{
    GlobalLog().WriteTime();
    GlobalLog().WriteString( "[EXCEPTION]: " );
    GlobalLog().WriteString( Message );
    GlobalLog().AddLine();
    GlobalLog().Flush();
    
    throw std::runtime_error( Message );
}
