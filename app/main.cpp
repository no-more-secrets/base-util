/****************************************************************
* Test driver
****************************************************************/
#include "base-util/logger.hpp"
#include "base-util/main.hpp"

using namespace std;
using util::operator<<;

int main_( int /*unused*/, char** /*unused*/ )
{
    util::Logger::enabled = true;

    util::log << "finished.\n";

    return 0;
}
