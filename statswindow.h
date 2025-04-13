#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class QTimer;

class StatsWindow : public QWidget {
    Q_OBJECT

public:
    StatsWindow(QWidget *parent = nullptr);

private slots:
    void update_data();

private:
    QTimer *timer;
    QChartView *chartView;
    QLineSeries *cpuSeries;
    QLineSeries *memSeries;
    int elapsedSeconds;
    void handleCloseButton();

};

#endif // STATSWINDOW_H
