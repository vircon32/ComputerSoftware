// *****************************************************************************
    // start include guard
    #ifndef LOGSTREAM_HPP
    #define LOGSTREAM_HPP
    
    // include project headers
    #include "Definitions.hpp"
    
    // include C/C++ headers
    #include <string>		      // [ C++ STL ] Strings
    #include <fstream>            // [ C++ STL ] File Streams
    #include <stdexcept>          // [ C++ STL ] Exceptions
// *****************************************************************************


// =============================================================================
//     TEXT OUTPUT TO A LOG FILE
// =============================================================================


class LogStream
{
    private:
        
        std::ofstream OutputFile;
        bool IndentPending;
        bool OpenBracePending;
        
    private:
        
        // internal auxiliary methods
        std::ostream& Target( void );
        
    public:
        
        bool ConsoleMode;
        unsigned char Indent;
    
    public:
        
        // instance handling
        LogStream();
       ~LogStream();
        
        // file handling methods
        bool OpenFile( const std::string& File, const std::string& Title );
        void CloseFile();
        
        // auxiliary methods
        void WriteIndent();
        void WriteDate();
        void WriteTime();
        void AddLine();
        void Flush();
        
        // scope handling
        void OpenScope();
        void CloseScope();
        
        // stream output for all common types
        LogStream& operator<<( const std::string& Message );     // text based types
        LogStream& operator<<( const char*   Message );
        LogStream& operator<<( char Character );
        
        LogStream& operator<<( short Number );              // signed integers
        LogStream& operator<<( int   Number );
        LogStream& operator<<( long  Number );
        
        LogStream& operator<<( unsigned char  Number );     // unsigned integers
        LogStream& operator<<( unsigned short Number );
        LogStream& operator<<( unsigned int   Number );
        LogStream& operator<<( unsigned long  Number );
        
        LogStream& operator<<( float  Number );             // floating point numbers
        LogStream& operator<<( double Number );
        
        LogStream& operator<<( void* Pointer );             // other common types
        LogStream& operator<<( bool Condition );
        
        // output of unformatted data
        void WriteByte( unsigned char Data );
        void WriteBuffer( const void* Buffer, unsigned long Bytes );
        
        // automation for common entry formats
        void NormalEntry( const std::string& Entry );
        void  TimedEntry( const std::string& Entry );
};


// =============================================================================
//     DEBUG DEFINITIONS AND TOOLS
// =============================================================================


// definition of new line character sequence
#define EndOfLine char('\n')

// macro to write a standard entry line in the global log
#define LOG( message )                          \
do                                              \
{                                               \
    GlobalLog() << message << EndOfLine;          \
    GlobalLog().Flush();                        \
}                                               \
while( 0 )

// macro to write an entry line with the current time
#define LOG_TIME( message )                     \
do                                              \
{                                               \
    GlobalLog().WriteTime();                    \
    GlobalLog() << message << EndOfLine;          \
    GlobalLog().Flush();                        \
}                                               \
while( 0 )

// automates writing results of executed instructions
#define LOG_RESULT( operation )                 \
do                                              \
{                                               \
    LOG( #operation << " = " << (operation) );  \
}                                               \
while( 0 )

// macros to create local log contexts in functions
#define LOG_SCOPE( title )    LogScope Scope( title       , GlobalLog() )
#define LOG_THIS              LogScope Scope( __FUNCTION__, GlobalLog() )
#define LOG_METHOD            LogScope Scope( __METHOD__  , GlobalLog() )


// =============================================================================
//     WRAPPER FUNCTIONS FOR THE GLOBAL LOG
// =============================================================================


// wrapper function to access the global log file
LogStream& GlobalLog();

// wrapper functions to open and close the global log
// NOTE: they cannot be used with logs other than the global log
void LOG_TO_FILE( const std::string& Name );
void LOG_TO_CONSOLE();
void LOG_END();


// =============================================================================
//      WRAPPER FUNCTIONS USABLE FOR ANY LOG
// =============================================================================


// inline function to throw an exception
// (named in capitals for better in-code visibility)
[[ noreturn ]] void inline THROW( const std::string& Message, LogStream& Log = GlobalLog() )
{
    Log.WriteTime();
    Log << "[EXCEPTION]: " << Message << EndOfLine;
    Log.Flush();
    
    throw std::runtime_error( Message );
}


// =============================================================================
//     SCOPED LOGGING CONTEXTS TO SEPARATE LOG SECTIONS
// =============================================================================


class LogScope
{
    private:
        
        LogStream& TargetLog;
        
    public:
        
        // instance handling
        LogScope( const std::string& Title, LogStream& Log = GlobalLog() );
       ~LogScope();
};


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
