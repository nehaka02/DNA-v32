#include <QCoreApplication>
#include <iostream>
#include <string>
#include <cstdio>
#include "memory.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::cout.setf(std::ios::unitbuf);

    // Set up code that uses the Qt event loop here.
    // Call a.quit() or a.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to a.exec() or use the Non-Qt Plain C++ Application template.

    std::cout << "DNA Simulator Running..." << std::endl;

    if(argc < 3){
        std::cout << "Simulator requires command-line arguments" << std::endl << std::flush;
    }

    char* arg1 = argv[1];
    char switchKey = *arg1;
    int memoryAddress;

    Memory newMemory;
    memoryAddress = std::stoi(argv[2]);
    if(memoryAddress > 32767){
        std::cout << "Memory address is outside valid 32-bit range" << std::endl << std::flush;
        return 0;
    }

    memoryAddress %= 32768;

    switch(switchKey) {

        case 'W':{
            if(argc != 5){
                std::cout << "Write command format: W [memory address][data][pipeline stage]" << std::endl << std::flush;

            }
            int data = std::stoi(argv[3]);
            int pipelineStage = std::stoi(argv[4]);
            if(pipelineStage != '4'){
                std::cout << "Only M pipeline stagen can write to memory" << std::endl << std::flush;

            }

            std::cout << "Writing to memory" << std::endl << std::flush;
            newMemory.writeMemory(memoryAddress, data, pipelineStage);
            printf("%d was written to %d", data, memoryAddress);
            break;
        }
        case 'R':{
            if (argc!=4){
                std::cout << "Read command format: R [memory address][pipeline stage]" << std::endl << std::flush;
            }
            int pipelineStage = std::stoi(argv[3]);
            if(pipelineStage != '1' || pipelineStage != '4'){
                std::cout << "Only F and M pipeline stages can read from memory" << std::endl << std::flush;
            }
            std::cout << "Reading from memory" << std::endl << std::flush;
            int readValue = newMemory.readMemory(memoryAddress, pipelineStage);
            printf("%d was read from %d", readValue, memoryAddress);
            break;
        }
        case 'V':{
            if (argc!=3){
                std::cout << "View command format: V [memory address]" << std::endl << std::flush;
            }
            std::cout << "Viewing memory" << std::endl << std::flush;
            int viewedValue = newMemory.viewMemory(memoryAddress);
            printf("%d was viewed from %d", viewedValue, memoryAddress);
            break;
        }
        default:
            std::cout << "First argument must be 'W', 'R', or 'V" << std::endl;
    }

    return 0;
}
