#include <QCoreApplication>
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>


void single_clock_cycle(std::string instructionAddr);

int main2(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout << "DNA Pipeline Simulator Running..." << std::endl;

    std::ifstream file("assembly.txt");
    std::string instructionAddr;

    std::cout << "Looking for file in: "
              << std::filesystem::current_path() << std::endl;

    if (!file.is_open()) {
        std::cout << "Error: could not open file" << std::endl;
        return 1;
    }

    while (std::getline(file, instructionAddr)) {

        // Skip empty lines
        if (instructionAddr.empty()) continue;

        // Directly feed into pipeline
        single_clock_cycle(instructionAddr);
    }

    file.close();

    // Flush pipeline
    // TODO


    return 0;
}
