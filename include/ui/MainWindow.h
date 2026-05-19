#pragma once
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include "MonitorWindow.h"

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private:
    void createActions();
    void createMenus();
    void createStatusBar();

    class QVBoxLayout* m_mainLayout;

    QMenuBar* m_menuBar;
    QStatusBar* m_statusBar;
    MonitorWindow* m_monitorWindow;

    QMenu* m_fileMenu;
    QMenu* m_helpMenu;
    QAction* m_exitAction;
    QAction* m_aboutAction;
};