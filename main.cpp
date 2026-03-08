#include <QCoreApplication>
#include <iostream>
#include <string>
#include <sstream>
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
void parseInput(std::vector<std::string> tokens, Cache* newCache) {
    char switchKey = tokens[0][0];
    int inputSize = tokens.size();

    switch(switchKey) {
        case 'W':{
            if(inputSize != 4){
                std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
            }
            int data = std::stoi(tokens[2]);
            int pipelineStage = std::stoi(tokens[3]);

            int memoryAddress = std::stoi(tokens[1]);
            std::cout << "Writing to memory..." << std::endl;
            while(newCache->writeMemory(memoryAddress, data, pipelineStage) != "Done"){
                newCache->clock++;
            }
            printf("%d was written to %d", data, memoryAddress);
            break;
        }

        case 'R':{
            if (inputSize!=3){
                std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
            }
            int pipelineStage = std::stoi(tokens[2]);

            std::cout << "Reading from memory..." << std::endl;
            int memoryAddress = std::stoi(tokens[1]);
            std::string readValue;
            while((readValue = newCache->readMemory(memoryAddress, pipelineStage)) != "Done"){
                newCache->clock++;
            }
            printf("%s was read from %d", readValue.c_str(), memoryAddress);
            break;
        }

        case 'V':{
            if (inputSize!=2){
                std::cout << "View command format: V [memory address]" << std::endl;
            }
            std::cout << "Viewing memory..." << std::endl;
            int memoryAddress = std::stoi(tokens[1]);
            int viewedValue = newCache->viewMemory(memoryAddress);
            printf("%d was viewed from %d", viewedValue, memoryAddress);
            break;
        }
        default:
            std::cout << "Command Menu:" << std::endl;
            std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
            std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
            std::cout << "View command format: V [memory address]" << std::endl;
    }

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout.setf(std::ios::unitbuf);

    std::cout << "DNA Simulator Running..." << std::endl;

    if(argc < 3){
        std::cout << "Command Menu:" << std::endl;
        std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
        std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
        std::cout << "View command format: V [memory address]" << std::endl;
    }

    char* arg1 = argv[1];
    char switchKey = *arg1;
    int memoryAddress;

    Memory newMemory;
    Cache* newCache = new Cache(&newMemory);
    memoryAddress = std::stoi(argv[2]);

    while (1) {
        std::string userInput;

        std::cout << "Enter command: ";

        // Read the entire line from standard input (cin) into the string
        std::getline(std::cin, userInput);

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
    // char* arg1 = argv[1];
    // char switchKey = *arg1;
    // int memoryAddress;

    // Memory newMemory;
    // Cache* newCache = new Cache(&newMemory);
    // memoryAddress = std::stoi(argv[2]);

    // switch(switchKey) {

    //     case 'W':{
    //         if(argc != 5){
    //             std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
    //         }
    //         int data = std::stoi(argv[3]);
    //         int pipelineStage = std::stoi(argv[4]);

    //         std::cout << "Writing to memory..." << std::endl;
    //         while(newCache->writeMemory(memoryAddress, data, pipelineStage) != "Done"){
    //             newCache->clock++;
    //         }
    //         printf("%d was written to %d", data, memoryAddress);
    //         break;
    //     }

    //     case 'R':{
    //         if (argc!=4){
    //             std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
    //         }
    //         int pipelineStage = std::stoi(argv[3]);

    //         std::cout << "Reading from memory..." << std::endl;
    //         std::string readValue;
    //         while((readValue = newCache->readMemory(memoryAddress, pipelineStage)) != "Done"){
    //             newCache->clock++;
    //         }
    //         printf("%s was read from %d", readValue.c_str(), memoryAddress);
    //         break;
    //     }

    //     case 'V':{
    //         if (argc!=3){
    //             std::cout << "View command format: V [memory address]" << std::endl;
    //         }
    //         std::cout << "Viewing memory..." << std::endl;
    //         int viewedValue = newCache->viewMemory(memoryAddress);
    //         printf("%d was viewed from %d", viewedValue, memoryAddress);
    //         break;
    //     }
    //     default:
    //         std::cout << "Command Menu:" << std::endl;
    //         std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl;
    //         std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl;
    //         std::cout << "View command format: V [memory address]" << std::endl;
    // }

    return 0;
}
