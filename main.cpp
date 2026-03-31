#include <QCoreApplication>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdio>
#include "cache.h"

/*
Memory is an array of integers, not bits.
Cache must be an object with a pointer to memory.
Use a single delay counter to service one access at a time.
There is no sleeping/asynchronous behavior -- delay is simulated by repeatedly issuing W/R until counter goes to 0.
The repeated issuing of W/R is handled by driver, not user.
RAM should have 4-words in a line if cache has 4-words in a line.
*/

// 2. This version is for reading from file instead of command line

std::string parseInput(std::vector<std::string> tokens, Cache* newCache) {
    char switchKey = tokens[0][0];
    int inputSize = tokens.size();

    // Print out the command read
    std::cout << "Command: ";
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        std::cout << tokens[i];
        if (i < tokens.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;

    switch(switchKey) {
        case 'W':{
            int data = std::stoi(tokens[2]);
            int pipelineStage = std::stoi(tokens[3]);

            int memoryAddress = std::stoi(tokens[1]);
            std::cout << "Writing to memory..." << std::endl;
            while(newCache->writeMemory(memoryAddress, data, pipelineStage) != "Done"){
                newCache->clock++;
            }
            //printf("%d was written to %d, current clock cycle = \n", data, memoryAddress);
            std::cout << data << " was stored to cache and RAM, address 0b"
                      << std::bitset<15>(memoryAddress) << std::endl;
            std::cout << "Clock = " << newCache->clock << std::endl;
            std::cout << std::endl;
            return 0;
        }

        case 'R':{

            int pipelineStage = std::stoi(tokens[2]);

            std::cout << "Reading from memory..." << std::endl;
            int memoryAddress = std::stoi(tokens[1]);
            std::string readValue;

            while(true){
                readValue = newCache->readMemory(memoryAddress, pipelineStage);
                if (readValue.rfind("Done:", 0) == 0) {  // starts with "Done:"
                    break;
                }
                newCache->clock++;
            }
            //printf("%s was read from %d\n", readValue.c_str(), memoryAddress);
            std::cout << readValue.c_str() << " was read from memory, address 0b"
                      << std::bitset<15>(memoryAddress) << std::endl;
            std::cout << "Clock = " << newCache->clock << std::endl;
            std::cout << std::endl;
            std::string readValSub = readValue.substr(6);
            return readValSub;

        }
        // Needs to support view cache memory and view RAM (fo ram limit view range)
        // if location to view = 0 viewing RAM, else location to view = 1 viewing cache
        case 'V':{
            if (std::stoi(tokens[1]) == 1) {
                if (inputSize == 2){
                    std::cout << "View cache..." << std::endl;
                    newCache->printCache();
                }
                else if (inputSize == 3){
                    std::cout << "View cache block..." << std::endl;
                    int memoryAddress = std::stoi(tokens[1]);
                    int viewedValue = newCache->viewMemory(memoryAddress);
                    //printf("%d was viewed from cache, address%d\n", viewedValue, memoryAddress);
                    std::cout << viewedValue << " was viewed from cache, address "
                              << std::bitset<15>(memoryAddress) << std::endl;
                    std::cout << std::endl;
                }
            }

            else if (std::stoi(tokens[1]) == 0 && inputSize == 4){
                int startLine = std::stoi(tokens[2]);
                int endLine = std::stoi(tokens[3]);
                std::cout << "View RAM..." << "start line: " << startLine << ", end line: "<< endLine << std::endl;
                newCache->printMemory(startLine, endLine);
            }
            return 0;
        }
        default:
            std::cout << "Invalid command read from file :(" << std::endl;
            std::cout << std::endl;
            return 0;
    }

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "DNA Simulator Running..." << std::endl;

    std::cout << "Command Formats" << std::endl;
    std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
    std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
    std::cout << "View cache command format: V [1]" << std::endl;
    std::cout << "View cache block command format: V [1] [memory address]" << std::endl;
    std::cout << "View RAM range command format: V [0] [start line] [end line]" << std::endl;
    std::cout << "\n" << std::endl;

    Memory newMemory;
    Cache* newCache = new Cache(&newMemory);

    // For reading from a file
    std::ifstream file("demo_commands.txt");
    std::string userInput;

    // Make sure to set working directory to DNA-v32
    std::cout << "Looking for file in: " << std::filesystem::current_path() << std::endl;
    if (!file.is_open()) {
        std::cout << "Error: could not open file" << std::endl;
        return 1;
    }

    while (std::getline(file, userInput)) {

        // Read the entire line from file into the string
        std::istringstream input(userInput);
        std::vector<std::string> tokens;
        std::string token;

        // Read tokens separated by a space
        while (input >> token) {
            tokens.push_back(token);
        }

        // parseInput logic
        parseInput(tokens, newCache);

    }

    return 0;
}
