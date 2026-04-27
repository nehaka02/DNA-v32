#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "memory.h"
#include "cache.h"
#include "pipeline.h"
#include "registers.h"
#include <QLabel>
#include <QSet>
#include <QTextEdit>
#include <QPushButton>
#include <QDockWidget>


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
    void onTogglePipeline();
    void onLoadAssemblyFile();
    void onAssemble();

private:
    Ui::MainWindow *ui;

    QDockWidget* m_memCacheDock;
    Memory*   m_memory   = nullptr;
    Cache*    m_cache    = nullptr;
    Pipeline* m_pipeline = nullptr;
    bool      m_machineActive = false;
    /*int m_breakpoint = -1;*/ // -1 means no breakpoint
    QSet<int> m_breakpoints;
    QLabel* m_breakpointLabel = nullptr;
    QLabel* m_clockLabel = nullptr;
    bool cacheEnabled = true;
    bool pipelineEnabled = true;

    void initSimulator();
    void runLoop();
    void refresh();
    void refreshRegisters();
    void refreshMemory();
    void refreshCache();
    void refreshPipeline();
    //void refreshPendingRegs();
    void refreshFlags();
    void showEvent(QShowEvent* event) override;

};

#endif // MAINWINDOW_H
