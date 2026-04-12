#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "driver.h"
#include "parseInput.h"
#include <QTableWidgetItem>
#include <QColor>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <QHeaderView>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    centralWidget()->hide();
    resize(1200, 800);

    connect(ui->actionRun,   &QAction::triggered, this, &MainWindow::onRun);
    connect(ui->actionStep,  &QAction::triggered, this, &MainWindow::onStep);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onReset);
    connect(ui->actionsetBreakpoint, &QAction::triggered, this, &MainWindow::onSetBreakpoint);
    connect(ui->actionclearBreakpoint, &QAction::triggered, this, &MainWindow::onClearBreakpoint);

    m_breakpointLabel = new QLabel("Breakpoint: none");
    ui->toolBar->addWidget(m_breakpointLabel);

    initSimulator();

    // Four quadrant layout
    addDockWidget(Qt::TopDockWidgetArea,    ui->pipelineDock);
    addDockWidget(Qt::TopDockWidgetArea,    ui->registerDock);
    addDockWidget(Qt::BottomDockWidgetArea, ui->memoryDock);
    addDockWidget(Qt::BottomDockWidgetArea, ui->cacheDock);

    // Split top and bottom into left/right
    splitDockWidget(ui->pipelineDock, ui->registerDock, Qt::Horizontal);
    splitDockWidget(ui->memoryDock,   ui->cacheDock,    Qt::Horizontal);

    // Lock docks in place
    ui->pipelineDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->registerDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->memoryDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->cacheDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pipeline;
    delete m_cache;
    delete m_memory;
}

void MainWindow::initSimulator()
{
    delete m_pipeline; m_pipeline = nullptr;
    delete m_cache;    m_cache    = nullptr;
    delete m_memory;   m_memory   = nullptr;

    m_memory = new Memory();
    m_cache  = new Cache(m_memory);
    m_machineActive = true;


    // Clear all registers
    for (int i = 0; i < 16; i++) {
        intRegs.r[i] = 0;
    }

    std::ifstream file("demo_commands2.txt");
    if (!file.is_open()) {
        std::cout << "Error: could not open demo_commands2.txt" << std::endl;
        return;
    }

    std::string userInput;
    while (std::getline(file, userInput)) {
        std::istringstream input(userInput);
        std::vector<std::string> tokens;
        std::string token;
        while (input >> token) tokens.push_back(token);
        if (!tokens.empty()) parseInput(tokens, m_cache);
    }

    m_pipeline = new Pipeline(m_cache);
    std::cout << "Simulator initialized." << std::endl;
}

void MainWindow::onRun()
{
    if (!m_pipeline || !m_machineActive) return;
    runLoop();
}

void MainWindow::onStep()
{
    if (!m_pipeline || !m_machineActive) return;

    single_clock_cycle(m_pipeline);

    if (m_pipeline->wInstr.opcode == 5) {
        m_machineActive = false;
        std::cout << "HALT encountered." << std::endl;
    }
    if (intRegs.r[13] >= 8192) m_machineActive = false;

    refresh();
}

void MainWindow::onReset()
{
    initSimulator();
    refresh();
}

// void MainWindow::onSetBreakpoint()
// {
//     bool ok;
//     int address = QInputDialog::getInt(
//         this,
//         "Set Breakpoint",
//         "Enter PC address:",
//         m_breakpoint,  // default value
//         0,             // min
//         8192,          // max
//         1,             // step
//         &ok
//         );
//     if (ok) {
//         m_breakpoint = address;
//         m_breakpointLabel->setText(QString("Breakpoint: PC=%1").arg(m_breakpoint));
//         std::cout << "Breakpoint set at PC=" << m_breakpoint << std::endl;
//     }
// }

void MainWindow::onSetBreakpoint()
{
    bool ok;
    int address = QInputDialog::getInt(
        this, "Set Breakpoint", "Enter PC address:",
        0, 0, 8192, 1, &ok
        );
    if (ok) {
        m_breakpoints.insert(address);
        QStringList list;
        for (int bp : m_breakpoints) list.append(QString::number(bp));
        m_breakpointLabel->setText("Breakpoints: " + list.join(", "));
    }
}

// void MainWindow::onClearBreakpoint()
// {
//     m_breakpoint = -1;
//     m_breakpointLabel->setText("Breakpoint: none");
// }

void MainWindow::onClearBreakpoint()
{
    m_breakpoints.clear();
    m_breakpointLabel->setText("Breakpoints: none");
}

// void MainWindow::runLoop()
// {
//     int i = 0;
//     while (m_machineActive && i < 30) {
//         single_clock_cycle(m_pipeline);

//         if (m_pipeline->wInstr.opcode == 5) {
//             m_machineActive = false;
//             std::cout << "HALT encountered." << std::endl;
//         }
//         if (intRegs.r[13] >= 8192) m_machineActive = false;

//         i++;
//     }
//     refresh();
// }

// void MainWindow::runLoop()
// {
//     int i = 0;
//     while (m_machineActive && i < 30) {
//         single_clock_cycle(m_pipeline);

//         if (m_pipeline->wInstr.opcode == 5) {
//             m_machineActive = false;
//             std::cout << "HALT encountered." << std::endl;
//         }
//         if (intRegs.r[13] >= 8192) m_machineActive = false;

//         // Check breakpoint
//         // if (m_breakpoint != -1 && intRegs.r[13] == m_breakpoint) {
//         //     std::cout << "Breakpoint hit at PC=" << m_breakpoint << std::endl;
//         //     break;
//         // }
//         if (m_breakpoints.contains(intRegs.r[13])) {
//             std::cout << "Breakpoint hit at PC=" << intRegs.r[13] << std::endl;
//             break;
//         }

//         i++;
//     }
//     refresh();
// }
void MainWindow::runLoop()
{
    int i = 0;
    while (m_machineActive && i < 30) {
        single_clock_cycle(m_pipeline);
        i++;

        if (m_pipeline->wInstr.opcode == 5) {
            m_machineActive = false;
            std::cout << "HALT encountered." << std::endl;
            break;
        }
        if (intRegs.r[13] >= 8192) {
            m_machineActive = false;
            break;
        }

        // Only check breakpoint after at least one cycle
        if (m_breakpoints.contains(intRegs.r[13])) {
            std::cout << "Breakpoint hit at PC=" << intRegs.r[13] << std::endl;
            break;
        }
    }
    refresh();
}

void MainWindow::refresh()
{
    refreshRegisters();
    refreshCache();
    refreshMemory();
    refreshPipeline();
}

void MainWindow::refreshRegisters()
{
    ui->IntRegisterTable->setRowCount(16);
    for (int i = 0; i < 16; i++) {
        ui->IntRegisterTable->setItem(i, 0, new QTableWidgetItem(QString("r%1").arg(i)));
        ui->IntRegisterTable->setItem(i, 1, new QTableWidgetItem(QString::number(intRegs.r[i])));
    }
    ui->IntRegisterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->VecRegisterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::refreshCache()
{
    ui->cacheTable->setRowCount(8);
    for (int i = 0; i < 8; i++) {
        ui->cacheTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        ui->cacheTable->setItem(i, 1, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][0])));
        ui->cacheTable->setItem(i, 2, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][2])));
        ui->cacheTable->setItem(i, 3, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][3])));
        ui->cacheTable->setItem(i, 4, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][4])));
        ui->cacheTable->setItem(i, 5, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][5])));
        ui->cacheTable->setItem(i, 6, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][6])));
        ui->cacheTable->setItem(i, 7, new QTableWidgetItem(QString::number(m_cache->cache_memory[i][7])));
    }
    ui->cacheTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::refreshMemory()
{
    int linesToShow = 32;
    ui->memoryTable->setRowCount(linesToShow);
    for (int i = 0; i < linesToShow; i++) {
        ui->memoryTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        ui->memoryTable->setItem(i, 1, new QTableWidgetItem(QString::number(m_cache->memory->dram[i][0])));
        ui->memoryTable->setItem(i, 2, new QTableWidgetItem(QString::number(m_cache->memory->dram[i][1])));
        ui->memoryTable->setItem(i, 3, new QTableWidgetItem(QString::number(m_cache->memory->dram[i][2])));
        ui->memoryTable->setItem(i, 4, new QTableWidgetItem(QString::number(m_cache->memory->dram[i][3])));
    }
    ui->memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::refreshPipeline()
{
    ui->pipelineTable->setColumnCount(1);

    const InstructionObject* stages[5] = {
        &m_pipeline->fInstr,
        &m_pipeline->dInstr,
        &m_pipeline->eInstr,
        &m_pipeline->mInstr,
        &m_pipeline->wInstr
    };

    for (int i = 0; i < 5; i++) {
        const auto& instr = *stages[i];
        QString text;
        QColor  color;

        if (instr.is_squashed) {
            text  = "SQUASHED";
            color = Qt::red;
        } else if (instr.is_stalled) {
            text  = "BUBBLE";
            color = Qt::gray;
        } else if (instr.is_blocked) {
            text  = QString("BLOCKED op=%1").arg(instr.opcode);
            color = Qt::yellow;
        } else if (instr.bin_instr == -1) {
            text  = "EMPTY";
            color = Qt::white;
        } else {
            text  = QString("op=%1 type=%2").arg(instr.opcode).arg(instr.type_code);
            color = Qt::green;
        }

        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setBackground(color);
        item->setTextAlignment(Qt::AlignCenter);
        ui->pipelineTable->setItem(i, 0, item);
    }
    ui->pipelineTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    int halfWidth  = width() / 2;
    int halfHeight = height() / 2;

    resizeDocks(
        {ui->pipelineDock, ui->registerDock},
        {halfWidth, halfWidth},
        Qt::Horizontal
        );
    resizeDocks(
        {ui->memoryDock, ui->cacheDock},
        {halfWidth, halfWidth},
        Qt::Horizontal
        );
    resizeDocks(
        {ui->pipelineDock, ui->memoryDock},
        {halfHeight, halfHeight},
        Qt::Vertical
        );
}
