#include "statswindow.h"
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QVBoxLayout>
#include <QLabel>
#include <fstream>
#include <sstream>
#include <QPushButton>
#include <QHBoxLayout>

using namespace QtCharts;
QT_CHARTS_USE_NAMESPACE


StatsWindow::StatsWindow(QWidget *parent) : QWidget(parent), elapsedSeconds(0) {
    setWindowTitle("CPU and Memory Usage Stats");
    resize(600, 400);

    cpuSeries = new QLineSeries();
    memSeries = new QLineSeries();

    QChart *chart = new QChart();
    chart->addSeries(cpuSeries);
    chart->addSeries(memSeries);
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).first()->setTitleText("Time (seconds)");
    chart->axes(Qt::Vertical).first()->setTitleText("Usage (%)");
    cpuSeries->setName("CPU Usage");
    memSeries->setName("Memory Usage");
    chart->setTitle("CPU and Memory Usage Over Time");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 🆕 Create a close button
    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);

    // 🆕 Layout with chart + button
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(chartView);
    layout->addWidget(closeButton, 0, Qt::AlignRight);  // Right-align the button
    setLayout(layout);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &StatsWindow::update_data);
    timer->start(2000);
}
void StatsWindow::handleCloseButton() {
    this->close();
}

void StatsWindow::update_data() {
    float cpuUsage = 0.0, memUsage = 0.0;
    std::ifstream statFile("/proc/stat");
    std::string line;
    if (std::getline(statFile, line)) {
        std::istringstream ss(line);
        std::string cpu;
        long user, nice, system, idle;
        ss >> cpu >> user >> nice >> system >> idle;
        long total = user + nice + system + idle;
        cpuUsage = 100.0f * (total - idle) / total;
    }

    std::ifstream meminfo("/proc/meminfo");
    float totalMem = 0, availableMem = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal") == 0) {
            std::istringstream ss(line);
            std::string key;
            ss >> key >> totalMem;
        }
        if (line.find("MemAvailable") == 0) {
            std::istringstream ss(line);
            std::string key;
            ss >> key >> availableMem;
            break;
        }
    }

    if (totalMem > 0) {
        memUsage = 100.0f * (totalMem - availableMem) / totalMem;
    }

    cpuSeries->append(elapsedSeconds, cpuUsage);
    memSeries->append(elapsedSeconds, memUsage);
    QChart *chart = chartView->chart();
    chart->axisX()->setRange(0, elapsedSeconds); // Expand time axis
    chart->axisY()->setRange(0, 100);  
    elapsedSeconds += 2;
}
