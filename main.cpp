#include <iostream>
#include <fstream>

#include "VoxelGraph.hpp"

#define help std::cout << "help main\n"

int main()
{
    // data to stream
    std::string data = "worlds/fortress.vox";
    std::ifstream stream(data);
    if (stream.fail())
    {
        std::cerr << "ERROR: Could not open file: " << data << '\n';
        return 1;
    }
    std::cout << "\nFilepath: " << data << '\n';

    // stream to VoxelGraph
    auto start = std::chrono::high_resolution_clock::now(); // TODO start chrono
    VoxelGraph map(stream);
    auto end = std::chrono::high_resolution_clock::now(); // TODO end chrono
    std::chrono::duration<double, std::micro> elapsed = end - start;
    std::cout << "Parse time: " << elapsed.count() << " microseconds\n\n";
    stream.close();

    // set source and target coordinates
    const Coordinate source(4, 2, 1), target(63, 41, 37);

    // find path
    start = std::chrono::high_resolution_clock::now(); // TODO start chrono
    try
    {
        Path path = map.find_path(source, target);
        std::cout << "Path length: " << path.size() << " steps \nPath: " << path << "\n"
                  //<< "eeeseessssseswwwwwneneneeennnesssssswwwsseeeeennnneeeeeeeenneeenneneeeeeesssssswnwwwswwwwswsseeeesssssssssessssessseseswwwwwnnnnnnneeeseeessssswnnnnnessssswnwnnennessssssenneeeeeeeeeneeennnneeeeeeessseeeeseeeeeneeeeeennwneeeeessswwnnwwsswswwnwnwwswwnwwswwwwwwwnnwnnnnennnnnwnnnnnneennwwwnwwnnneeeeeesssssswnnnnnessssssswwssswssswswssswwwwssssssssssseeeesssesswssssesseseeessseeeseeeenneeeeeseseeeeennnenessssswwnnennnwwswwwnnwnw"
                  << "\n";
    }
    catch (const InvalidCoordinate &err)
    {
        std::cout << "Invalid coordinate: " << err.coordinate << '\n';
    }
    catch (const Untraversable &err)
    {
        std::cout << "No path from " << err.source << " to " << err.target << ".\n";
    }
    end = std::chrono::high_resolution_clock::now(); // TODO end chrono
    elapsed = end - start;
    std::cout << "Search time: " << elapsed.count() << " microseconds\n";

    // reverse-find path
    start = std::chrono::high_resolution_clock::now(); // TODO start chrono
    try
    {
        Path path = map.find_path_reverse(source, target);
        std::cout << "Path length: " << path.size() << " steps \nPath: " << path.backwards() << "\n";
    }
    catch (const InvalidCoordinate &err)
    {
        std::cout << "Invalid coordinate: " << err.coordinate << '\n';
    }
    catch (const Untraversable &err)
    {
        std::cout << "No path from " << err.source << " to " << err.target << ".\n";
    }
    end = std::chrono::high_resolution_clock::now(); // TODO end chrono
    elapsed = end - start;
    std::cout << "Search time: " << elapsed.count() << " microseconds\n";

    // end program
    std::cout << std::endl;
    return 0;
}