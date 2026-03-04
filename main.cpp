#include <QCoreApplication>
#include <iostream>

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

    if(argc < 2){
        std::cout << "Simulator requires command-Line arguments" << std::endl << std::flush;
    }

    char* arg1 = argv[1];
    char switchKey = *arg1;

    switch(switchKey) {
        case 'W':
            std::cout << "Writing to memory" << std::endl << std::flush;
            break;
        case 'R':
            std::cout << "Reading from memory" << std::endl << std::flush;
            break;
        case 'V':
            std::cout << "Viewing memory" << std::endl << std::flush;
        default:
            std::cout << "First argument must be 'W', 'R', or 'V" << std::endl;
    }

    return 0;
}
