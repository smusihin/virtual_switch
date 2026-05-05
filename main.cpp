#include "IO.h"
#include "NetworkSwitch.h"

#include <iostream>
#include <vector>


int main( int argc, char** argv)
{
    std::vector<std::string_view> interfaces;
    interfaces.reserve(argc - 1);
    for (int i = 1; i < argc; ++i)
    {
        interfaces.push_back(argv[i]);
    }

    IO io(interfaces);
    NetworkSwitch networkSwitch(io, interfaces.size());
    networkSwitch.run();

    return 0;
}
