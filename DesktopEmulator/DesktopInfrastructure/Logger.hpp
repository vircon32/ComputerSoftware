// *****************************************************************************
    // start include guard
    #ifndef LOGGER_HPP
    #define LOGGER_HPP
    
    // include C/C++ headers
    #include <string>		      // [ C++ STL ] Strings
    #include <fstream>            // [ C++ STL ] File Streams
// *****************************************************************************


// =============================================================================
//     TEXT OUTPUT TO A LOG FILE
// =============================================================================


class Logger
{
    private:
        
        std::ofstream OutputFile;
        
        // internal auxiliary methods
        std::ostream& Target( void );
        
    public:
        
        bool ConsoleMode;
    
    public:
        
        // instance handling
        Logger();
       ~Logger();
        
        // file handling methods
        bool OpenFile( const std::string& File, const std::string& Title );
        void CloseFile();
        
        // log methods
        void WriteString( const std::string& Message );
        void WriteDate();
        void WriteTime();
        void AddLine();
        void Flush();
};


// =============================================================================
//     WRAPPER FUNCTIONS FOR THE GLOBAL LOG
// =============================================================================


// access to the global log instance
Logger& GlobalLog();

// opening and closing the global log
void LOG_TO_FILE( const std::string& Name );
void LOG_TO_CONSOLE();
void LOG_END();

// general use funcions for the global log
void LOG( const std::string& Message );
[[ noreturn ]] void THROW( const std::string& Message );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
