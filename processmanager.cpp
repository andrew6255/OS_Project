#include "processmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QLabel>
#include <QProcess>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>  

#include <utility>
#include <fstream>
#include <sstream>
std::pair<float, float> getCpuAndMemoryUsage();

ProcessManager::ProcessManager(QWidget *parent)
    : QWidget(parent), darkMode(false), sortByCPU(true) {

    table = new QTableWidget(this);

    search = new QLineEdit(this);
    search->setPlaceholderText("Search here...");
    searchType = new QComboBox(this);
    searchType->addItems({"Name", "PID"});

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("Search by: "));
    searchLayout->addWidget(searchType);
    searchLayout->addWidget(search);

    cpuThresholdSpin = new QDoubleSpinBox(this);
    cpuThresholdSpin->setRange(0, 1000);
    cpuThresholdSpin->setSuffix(" s");
    cpuThresholdSpin->setPrefix("Min CPU: ");
    cpuThresholdSpin->setValue(0);

    memThresholdSpin = new QDoubleSpinBox(this);
    memThresholdSpin->setRange(0, 1e6);
    memThresholdSpin->setSuffix(" MB");
    memThresholdSpin->setPrefix("Min Mem: ");
    memThresholdSpin->setValue(0);

    QHBoxLayout *thresholdLayout = new QHBoxLayout;
    thresholdLayout->addWidget(cpuThresholdSpin);
    thresholdLayout->addSpacing(20);
    thresholdLayout->addWidget(memThresholdSpin);

    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"PID", "Name", "CPU Usage", "Memory (MB)"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    totalCpuLabel = new QLabel("Total CPU Usage: --", this);
    totalMemLabel = new QLabel("Total Memory Usage: --", this);
    cpuUsagePercentLabel = new QLabel("CPU %: --", this);
    memUsagePercentLabel = new QLabel("Memory %: --", this);

    modeButton = new QPushButton("Switch Dark/Light Mode", this);
    sortButton = new QPushButton("Switch sorting by CPU/Memory Usage", this);
    killButton = new QPushButton("Kill Selected Process", this);
    statsButton = new QPushButton("Stats and Graphs", this);

    priorityBox = new QSpinBox(this);
    priorityBox->setRange(-20,19);
    priorityBox->setValue(0);
    priorityBox->setPrefix("Priority: ");
    
    setPriorityButton = new QPushButton("Set process priority", this);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(modeButton);
    buttonLayout->addWidget(sortButton);
    buttonLayout->addWidget(killButton);
    buttonLayout->addWidget(statsButton);
    buttonLayout->addWidget(setPriorityButton);
    buttonLayout->addWidget(priorityBox);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(thresholdLayout);
    mainLayout->addWidget(table);
    mainLayout->addWidget(totalCpuLabel);
    mainLayout->addWidget(totalMemLabel);
    mainLayout->addWidget(cpuUsagePercentLabel);
    mainLayout->addWidget(memUsagePercentLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    connect(modeButton, &QPushButton::clicked, this, &ProcessManager::toggle_dark_light_mode);
    connect(sortButton, &QPushButton::clicked, this, &ProcessManager::toggle_sort_mode);
    connect(killButton, &QPushButton::clicked, this, &ProcessManager::kill_selected_process);
    connect(statsButton, &QPushButton::clicked, this, &ProcessManager::show_stats_window);
    connect(search, &QLineEdit::textChanged, this, &ProcessManager::update_process_list);
    connect(searchType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProcessManager::update_process_list);
    connect(cpuThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProcessManager::update_process_list);
    connect(memThresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProcessManager::update_process_list);
    connect(setPriorityButton, &QPushButton::clicked, this, &ProcessManager::set_process_priority);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ProcessManager::update_process_list);
    timer->start(2000);

    update_process_list();
}

void ProcessManager::update_process_list() {
    table->setRowCount(0);
    QDir proc("/proc");
    QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    float total_cpu = 0.0;
    float total_mem = 0.0;
    QList<QList<QVariant>> processList;

    QString query = search->text().trimmed();
    QString type = searchType->currentText().trimmed();
    float cpuMin = static_cast<float>(cpuThresholdSpin->value());
    float memMin = static_cast<float>(memThresholdSpin->value());

    for (const QString &pidStr : entries) {
        bool ok;
        int pid = pidStr.toInt(&ok);
        if (!ok) continue;

        QString statPath = "/proc/" + pidStr + "/stat";
        QFile statFile(statPath);
        if (!statFile.open(QIODevice::ReadOnly)) continue;
        QTextStream statStream(&statFile);
        QStringList stats = statStream.readLine().split(' ');
        statFile.close();

        if (stats.size() < 14) continue;
        float cpu = stats.value(13).toFloat() / sysconf(_SC_CLK_TCK);
        total_cpu += cpu;

        QString memPath = "/proc/" + pidStr + "/statm";
        QFile memFile(memPath);
        float mem = 0.0;
        if (memFile.open(QIODevice::ReadOnly)) {
            QTextStream memStream(&memFile);
            QStringList memStats = memStream.readLine().split(" ");
            if (memStats.size() > 1) {
                float rss = memStats[1].toFloat();
                float pageSize = sysconf(_SC_PAGESIZE) / 1024.0 / 1024.0;
                mem = rss * pageSize;
                total_mem += mem;
            }
            memFile.close();
        }

        QString name;
        QFile nameFile("/proc/" + pidStr + "/comm");
        if (nameFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&nameFile);
            name = in.readLine();
            nameFile.close();
        }

        if (cpu < cpuMin) continue;
        if (mem < memMin) continue;

        if (!query.isEmpty()) {
            if ((type == "Name" || type == "name") &&
                !name.contains(query, Qt::CaseInsensitive))
                continue;
            if ((type == "PID"  || type == "pid") &&
                QString::number(pid) != query)
                continue;
        }

        processList.append({pid, name, cpu, mem});
    }

    if (!processList.isEmpty()) {
        std::sort(processList.begin(), processList.end(), [this](const QList<QVariant> &a, const QList<QVariant> &b) {
            return sortByCPU ? a[2].toFloat() > b[2].toFloat() : a[3].toFloat() > b[3].toFloat();
        });

        table->setRowCount(processList.size());
        for (int row = 0; row < processList.size(); ++row) {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(processList[row][0].toInt())));
            table->setItem(row, 1, new QTableWidgetItem(processList[row][1].toString()));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(processList[row][2].toFloat(), 'f', 2)));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(processList[row][3].toFloat(), 'f', 2)));
        }
    }

    totalCpuLabel->setText("Total CPU Usage: " + QString::number(total_cpu, 'f', 2));
    totalMemLabel->setText("Total Memory Usage: " + QString::number(total_mem, 'f', 2) + " MB");

    auto [cpuPercent, memPercent] = getCpuAndMemoryUsage();
    cpuUsagePercentLabel->setText("CPU %: " + QString::number(cpuPercent, 'f', 2) + "%");
    memUsagePercentLabel->setText("Memory %: " + QString::number(memPercent, 'f', 2) + "%");
}

void ProcessManager::kill_selected_process() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (!selected.isEmpty()) {
        int row = table->row(selected.first());
        int pid = table->item(row, 0)->text().toInt();
        if (kill(pid, SIGKILL) == 0) {
            QMessageBox::information(this, "Success", "Process killed successfully.");
        } else {
            QMessageBox::warning(this, "Error", "Failed to kill process.");
        }
    }
}

void ProcessManager::toggle_dark_light_mode() {
    darkMode = !darkMode;
    update_theme();
}

void ProcessManager::toggle_sort_mode() {
    sortByCPU = !sortByCPU;
    update_process_list();
}

void ProcessManager::update_theme() {
    qApp->setStyleSheet(darkMode ?
        "QWidget { background-color: #2b2b2b; color: white; }"
        "QPushButton { background-color: #3c3c3c; color: white; }"
        :
        "");
}

void ProcessManager::show_stats_window() {
    if (!statsWindow) {
        statsWindow = new StatsWindow(this);
    }
    statsWindow->show();
}

std::pair<float, float> getCpuAndMemoryUsage() {
    float cpuUsage = 0.0f, memUsage = 0.0f;

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

    return {cpuUsage, memUsage};
}

void ProcessManager::set_process_priority() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (!selected.isEmpty()) {
        int row = table->row(selected.first());
        // get the row and from there, select item
        int pid = table->item(row,0)->text().toInt();
        // select from box
        int new_prio = priorityBox->value();
        // adjusting one process (PRIO_PROCESS)
        if (setpriority(PRIO_PROCESS, pid, new_prio) == 0) {
            // so if equals 0, it's a success
            QMessageBox::information(this, "Success", "Priority changed!");
        } else {
            QMessageBox::warning(this, "Fail", "Failed to change priority.");
        }
    } else { // if empty
        QMessageBox::warning(this, "Nothing selected", "Please select a process");
    }
}