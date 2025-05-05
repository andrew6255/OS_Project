#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>

#include "statswindow.h"



class ProcessManager : public QWidget {
    Q_OBJECT

public:
    ProcessManager(QWidget *parent = nullptr);

private slots:
    void update_process_list();
    void kill_selected_process();
    void toggle_dark_light_mode();
    void toggle_sort_mode();
    void update_theme();
    void show_stats_window();
    void set_process_priority();

private:
    QTableWidget *table;
    QPushButton *modeButton;
    QPushButton *sortButton;
    QPushButton *killButton;
    QLabel *totalCpuLabel;
    QLabel *totalMemLabel;
    QLabel *cpuUsagePercentLabel;
    QLabel *memUsagePercentLabel;
    QPushButton *statsButton;
    StatsWindow *statsWindow;
    QLineEdit *search;
    QComboBox *searchType;
    QDoubleSpinBox *cpuThresholdSpin;
    QDoubleSpinBox *memThresholdSpin;
    QSpinBox *priorityBox;
    QPushButton *setPriorityButton;

    bool darkMode;
    bool sortByCPU;
};

#endif
