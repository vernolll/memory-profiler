#include "../../include/ui/MainWindow.h"

#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_menuBar = new QMenuBar(this);
    m_mainLayout->addWidget(m_menuBar);

    createActions();
    createMenus();

    m_monitorWindow = new MonitorWindow(this);
    m_mainLayout->addWidget(m_monitorWindow, 1);

    createStatusBar();

    setWindowTitle("Core Memory Profiler v1.0");
    resize(1200, 700);
}

void MainWindow::createActions()
{
    m_exitAction = new QAction("Exit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_aboutAction = new QAction("About the program", this);
    connect(m_aboutAction, &QAction::triggered, this, [this]() 
        {
        QMessageBox::about(this, "About the program",
            "<h3>Core Memory Profiler v1.0</h3>"
            "<p>The tool is designed for high-speed allocation tracking "
            "in real time via Shared Memory with minimal overhead.</p>");
        });
}

void MainWindow::createMenus()
{
    m_fileMenu = m_menuBar->addMenu("File");
    m_fileMenu->addAction(m_exitAction);

    m_helpMenu = m_menuBar->addMenu("Help");
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::createStatusBar()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setStyleSheet("background-color: #252526; color: #aaa; border-top: 1px solid #333;");
    m_mainLayout->addWidget(m_statusBar);

    m_statusBar->showMessage("Ready to work");
}