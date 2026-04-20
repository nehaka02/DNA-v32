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
#include <QFileDialog>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QProcess>
#include <QTemporaryFile>

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
    connect(ui->actionToggleCache, &QAction::triggered, this, &MainWindow::onToggleCache);
    ui->actionToggleCache->setEnabled(true);

    m_breakpointLabel = new QLabel("Breakpoint: none");
    ui->toolBar->addWidget(m_breakpointLabel);

    m_clockLabel = new QLabel("Clock: 0");
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(m_clockLabel);

    connect(ui->loadFileBtn, &QPushButton::clicked,this, &MainWindow::onLoadAssemblyFile);
    connect(ui->assembleBtn, &QPushButton::clicked,this, &MainWindow::onAssemble);

    initSimulator();

    //Memory and Cache tabbed dock

    QTabWidget* memCacheTab = new QTabWidget();
    QWidget* memoryWidget = ui->memoryDock->widget();
    QWidget* cacheWidget  = ui->cacheDock->widget();
    memCacheTab->addTab(memoryWidget, "Memory");
    memCacheTab->addTab(cacheWidget,  "Cache");


    m_memCacheDock = new QDockWidget("Memory / Cache", this);
    m_memCacheDock->setWidget(memCacheTab);
    m_memCacheDock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    removeDockWidget(ui->memoryDock);
    removeDockWidget(ui->cacheDock);
    ui->memoryDock->deleteLater();
    ui->cacheDock->deleteLater();

    // Four quadrant layout
    addDockWidget(Qt::TopDockWidgetArea,    ui->pipelineDock);
    addDockWidget(Qt::TopDockWidgetArea,    ui->registerDock);
    addDockWidget(Qt::BottomDockWidgetArea, m_memCacheDock);
    //addDockWidget(Qt::BottomDockWidgetArea, ui->cacheDock);
    addDockWidget(Qt::BottomDockWidgetArea, ui->assemblerDock);

    // Split top and bottom into left/right
    splitDockWidget(ui->pipelineDock, ui->registerDock, Qt::Horizontal);
    //splitDockWidget(ui->memoryDock,   ui->cacheDock,    Qt::Horizontal);
    splitDockWidget(m_memCacheDock, ui->assemblerDock,  Qt::Horizontal);

    // Lock docks in place
    ui->pipelineDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->registerDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // ui->memoryDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // ui->cacheDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    ui->assemblerDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
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
    m_cache->cacheEnabled = cacheEnabled;

    m_machineActive = true;


    // Clear all registers
    for (int i = 0; i < 16; i++) {
        intRegs.r[i] = 0;
    }

    std::ifstream file("demos/demo_commands2.txt");
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
    ui->actionToggleCache->setEnabled(false);
    if (!m_pipeline || !m_machineActive) return;
    runLoop();
}

void MainWindow::onStep()
{
    ui->actionToggleCache->setEnabled(false);
    if (!m_pipeline || !m_machineActive) return;

    single_clock_cycle(m_pipeline, cacheEnabled);

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
    ui->actionToggleCache->setEnabled(true);
    refresh();
}


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

void MainWindow::onClearBreakpoint()
{
    m_breakpoints.clear();
    m_breakpointLabel->setText("Breakpoints: none");
}

void MainWindow::onToggleCache()
{
    cacheEnabled = !cacheEnabled;
    m_cache->cacheEnabled = cacheEnabled;
    ui->actionToggleCache->setText(cacheEnabled ? "Cache: ON" : "Cache: OFF");
}


void MainWindow::runLoop()
{
    ui->actionToggleCache->setEnabled(false);
    int i = 0;
    while (m_machineActive && i < 10000) {
        single_clock_cycle(m_pipeline, cacheEnabled);
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
    m_clockLabel->setText(QString("Clock: %1").arg(m_pipeline->global_clock));
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
        {m_memCacheDock, ui->assemblerDock},
        {halfWidth, halfWidth},
        Qt::Horizontal
        );
    resizeDocks(
        {ui->pipelineDock, m_memCacheDock},
        {halfHeight, halfHeight},
        Qt::Vertical
        );
    // resizeDocks(
    //     {ui->memoryDock, ui->cacheDock},
    //     {halfWidth, halfWidth},
    //     Qt::Horizontal
    //     );
    // resizeDocks(
    //     {ui->pipelineDock, ui->memoryDock},
    //     {halfHeight, halfHeight},
    //     Qt::Vertical
    //     );
}

void MainWindow::onLoadAssemblyFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Assembly File",
        "",
        "Assembly Files (*.s *.asm *.txt);;All Files (*)"
        );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->assembleStatus->setText("Failed to open file.");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    ui->assemblyInput->setPlainText(content);
    ui->assembleStatus->setText("Loaded: " + fileName);
}

void MainWindow::onAssemble()
{
    initSimulator();
    QString text = ui->assemblyInput->toPlainText();

    // Write file to temporary file to provide to python assembler
    QString assemblerPath = QDir::currentPath() + "/assembler/assembler.py";
    QString inputFile  = QDir::currentPath() + "/temp.s";
    QString outputFile = QDir::currentPath() + "/output.txt";
    //QString inputFile = "temp.s";
    QFile file(inputFile);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ui->assembleStatus->setText("Failed to write temp file.");
        return;
    }

    QTextStream out(&file);
    out << text;
    file.close();

    // Call python assembler

    QProcess process;
    process.start("python3", QStringList() << assemblerPath << inputFile << outputFile);
    process.waitForFinished();

    // QProcess process;
    // process.start("python", QStringList() << "/assembler/assembler.py" << inputFile << outputFile);
    // process.waitForFinished();

    QString stdErr = process.readAllStandardError();
    QString stdOut = process.readAllStandardOutput();
    qDebug() << "STDOUT:" << stdOut;
    qDebug() << "STDERR:" << stdErr;

    if (!stdErr.isEmpty()) {
        ui->assembleStatus->setText("Assembler error: " + stdErr);
        return;
    }

    if (process.exitStatus() != QProcess::NormalExit) {
        ui->assembleStatus->setText("Assembler failed.");
        return;
    }

    // Parse output file from python assembler
    // Output file is in the form W [address] [machine code] [pipeline stage = 4], basically what we have in the demo code

    // Double check that the file was created correctly
    if (!QFile::exists(outputFile)) {
        ui->assembleStatus->setText("Output file was NOT created.");
        return;
    }

    // Attempt to open and read the file
    QFile outFile(outputFile);
    if (!outFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->assembleStatus->setText("Failed to read output file.");
        return;
    }

    QTextStream in(&outFile);

    initSimulator();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty())
            continue;

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        std::vector<std::string> tokens;
        tokens.reserve(parts.size());

        for (const QString& p : parts) {
            tokens.push_back(p.toStdString());
        }

        // Load all lines to memory in order 0, 1, 2, ...
        parseInput(tokens, m_cache);
    }

    outFile.close();

    ui->assembleStatus->setText("Assembled and loaded successfully.");
    refresh();

}
