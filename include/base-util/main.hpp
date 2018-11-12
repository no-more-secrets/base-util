/****************************************************************
* main() stub
****************************************************************/
#pragma once

#include <exception>
#include <iostream>

// This module should only be included once per executable binary
// since it contains a main() module.

// User is expected to provide one of these.
int main_( int argc, char** argv );

// Entrypoint  of  program. This is just for convenience; it will
// catch all exceptions and display message if available.
// NOLINTNEXTLINE(misc-definitions-in-headers)
int main( int argc, char** argv ) {
    
    try {
        return main_( argc, argv );
    //} catch( sqlite::sqlite_exception const& e ) {
    //    std::cerr << sqlite::exception_msg( e ) << "\n";
    } catch( std::exception const& e ) {
        std::cerr << "exception: " << e.what() << "\n";
    } catch( ... ) {
        std::cerr << "exception: unknown.\n";
    }
    return 1;
}
