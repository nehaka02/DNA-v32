#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "memory.h"
#include "cache.h"
#include "pipeline.h"
#include "registers.h"
#include <QLabel>
#include <QSet>


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
    void onSetBreakpoint();
    void onClearBreakpoint();
    void onToggleCache();

private:
    Ui::MainWindow *ui;

    Memory*   m_memory   = nullptr;
    Cache*    m_cache    = nullptr;
    Pipeline* m_pipeline = nullptr;
    bool      m_machineActive = false;
    /*int m_breakpoint = -1;*/ // -1 means no breakpoint
    QSet<int> m_breakpoints;
    QLabel* m_breakpointLabel = nullptr;
    QLabel* m_clockLabel = nullptr;
     bool cacheEnabled = true;

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
