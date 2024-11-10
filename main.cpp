#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~main\n"

int main()
{
    std::ofstream outFile("outputs/travel_plans.txt", std::ios::binary);

    if (!outFile)
    {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }

    const char data[] = {0};
    outFile.write(data, 1);

    outFile.close();
    std::cout << "Data successfully written to file!" << std::endl;

    return 0;
}