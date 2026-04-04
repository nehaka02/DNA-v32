#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "memory.h"
#include "cache.h"
#include "pipeline.h"
#include "registers.h"

namespace Ui { class MainWindow; }

extern Registers::IntegerRegs intRegs;
extern Registers::PendIntegerRegs pendRegs;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRun();
    void onStep();
    void onReset();

private:
    Ui::MainWindow *ui;

    Memory*   m_memory   = nullptr;
    Cache*    m_cache    = nullptr;
    Pipeline* m_pipeline = nullptr;
    bool      m_machineActive = false;

    void initSimulator();
    void runLoop();
    void refresh();
    void refreshRegisters();
    void refreshMemory();
    void refreshCache();
    void refreshPipeline();
    void showEvent(QShowEvent* event) override;
};

#endif // MAINWINDOW_H
