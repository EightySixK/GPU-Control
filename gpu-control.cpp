#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QProcess>
#include <QFont>
#include <QFrame>
#include <QFile>
#include <QTextStream>
#include <QDir>

class GpuControl : public QWidget {
    Q_OBJECT

public:
    GpuControl(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("GPU Control");
        setMinimumSize(520, 780);

        maxPowerLimit = queryMaxPowerLimit();
        defaultPowerLimit = queryDefaultPowerLimit();

        QString spinStyle =
            "QSpinBox { min-height: 36px; min-width: 160px; font-size: 14px; padding: 4px 8px; }";

        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(14);
        mainLayout->setContentsMargins(20, 20, 20, 20);

        // Title
        auto *title = new QLabel("NVIDIA GPU Control");
        QFont titleFont = title->font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        title->setFont(titleFont);
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        // GPU name
        gpuNameLabel = new QLabel(queryGpuName());
        gpuNameLabel->setAlignment(Qt::AlignCenter);
        gpuNameLabel->setStyleSheet("color: #76b900; padding: 2px; font-size: 12px;");
        mainLayout->addWidget(gpuNameLabel);

        // Status
        statusLabel = new QLabel("Reading current values...");
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setStyleSheet("color: #888; padding: 6px; font-size: 13px;");
        mainLayout->addWidget(statusLabel);

        // Separator
        auto *sep1 = new QFrame();
        sep1->setFrameShape(QFrame::HLine);
        mainLayout->addWidget(sep1);

        // Power Limit
        auto *powerGroup = new QGroupBox("Power Limit");
        powerGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; }");
        auto *powerVBox = new QVBoxLayout(powerGroup);
        powerVBox->setContentsMargins(12, 16, 12, 12);
        powerVBox->setSpacing(8);
        auto *powerHBox = new QHBoxLayout();
        auto *powerLabel = new QLabel("Power Limit:");
        powerLabel->setStyleSheet("font-size: 13px;");
        powerSpin = new QSpinBox();
        powerSpin->setRange(100, maxPowerLimit);
        powerSpin->setValue(360);
        powerSpin->setSuffix(" W");
        powerSpin->setStyleSheet(spinStyle);
        powerHBox->addWidget(powerLabel);
        powerHBox->addStretch();
        powerHBox->addWidget(powerSpin);
        powerVBox->addLayout(powerHBox);

        // Power ratio label
        auto *powerRatioHBox = new QHBoxLayout();
        powerRatioLabel = new QLabel(QString("360 / %1 W").arg(maxPowerLimit));
        powerRatioLabel->setStyleSheet("font-size: 12px; color: #aaa;");
        powerRatioHBox->addWidget(powerRatioLabel);
        powerRatioHBox->addStretch();
        powerVBox->addLayout(powerRatioHBox);
        mainLayout->addWidget(powerGroup);

        // Memory Offset
        auto *memGroup = new QGroupBox("Memory Clock Offset");
        memGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; }");
        auto *memVBox = new QVBoxLayout(memGroup);
        memVBox->setContentsMargins(12, 16, 12, 12);
        memVBox->setSpacing(8);
        auto *memHBox = new QHBoxLayout();
        auto *memLabel = new QLabel("Offset:");
        memLabel->setStyleSheet("font-size: 13px;");
        memSpin = new QSpinBox();
        memSpin->setRange(-2000, 6000);
        memSpin->setValue(3000);
        memSpin->setSingleStep(100);
        memSpin->setSuffix(" MHz");
        memSpin->setStyleSheet(spinStyle);
        memHBox->addWidget(memLabel);
        memHBox->addStretch();
        memHBox->addWidget(memSpin);
        memVBox->addLayout(memHBox);
        auto *equivHBox = new QHBoxLayout();
        auto *equivNote = new QLabel("Afterburner equivalent:");
        equivNote->setStyleSheet("font-size: 12px; color: #aaa;");
        memEquiv = new QLabel("+1500");
        memEquiv->setStyleSheet("font-size: 12px; font-weight: bold; color: #aaa;");
        equivHBox->addWidget(equivNote);
        equivHBox->addWidget(memEquiv);
        equivHBox->addStretch();
        memVBox->addLayout(equivHBox);
        mainLayout->addWidget(memGroup);

        // Core Clock Offset
        auto *coreGroup = new QGroupBox("Core Clock Offset");
        coreGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; }");
        auto *coreLayout = new QHBoxLayout(coreGroup);
        coreLayout->setContentsMargins(12, 16, 12, 12);
        auto *coreLabel = new QLabel("Offset:");
        coreLabel->setStyleSheet("font-size: 13px;");
        coreSpin = new QSpinBox();
        coreSpin->setRange(-1000, 1000);
        coreSpin->setValue(0);
        coreSpin->setSingleStep(15);
        coreSpin->setSuffix(" MHz");
        coreSpin->setStyleSheet(spinStyle);
        coreLayout->addWidget(coreLabel);
        coreLayout->addStretch();
        coreLayout->addWidget(coreSpin);
        mainLayout->addWidget(coreGroup);

        // Presets
        auto *presetGroup = new QGroupBox("Presets");
        presetGroup->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; }");
        auto *presetLayout = new QHBoxLayout(presetGroup);
        presetLayout->setContentsMargins(12, 16, 12, 12);
        presetLayout->setSpacing(10);

        QString presetBtnStyle =
            "QPushButton { min-height: 50px; font-size: 12px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #444; }";

        auto *defaultBtn = new QPushButton(QString("Default\n%1W / +0").arg(defaultPowerLimit));
        auto *lowBtn = new QPushButton(QString("Low Power\n%1W / +0").arg((int)(defaultPowerLimit * 0.75)));
        auto *fullBtn = new QPushButton(QString("Full Power\n%1W / +0").arg(maxPowerLimit));
        defaultBtn->setStyleSheet(presetBtnStyle);
        lowBtn->setStyleSheet(presetBtnStyle);
        fullBtn->setStyleSheet(presetBtnStyle);
        presetLayout->addWidget(defaultBtn);
        presetLayout->addWidget(lowBtn);
        presetLayout->addWidget(fullBtn);
        mainLayout->addWidget(presetGroup);

        // Startup checkbox
        startupCheck = new QCheckBox("Apply on startup");
        startupCheck->setStyleSheet("font-size: 13px; padding: 4px;");
        mainLayout->addWidget(startupCheck);

        // Apply button
        auto *applyBtn = new QPushButton("Apply");
        applyBtn->setFixedHeight(48);
        applyBtn->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; font-size: 16px; font-weight: bold; border-radius: 8px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #3d8b40; }"
        );
        mainLayout->addWidget(applyBtn);

        // Reset button
        auto *resetBtn = new QPushButton("Reset to Defaults");
        resetBtn->setFixedHeight(36);
        resetBtn->setStyleSheet(
            "QPushButton { background-color: #555; color: #ccc; font-size: 13px; border-radius: 6px; }"
            "QPushButton:hover { background-color: #c0392b; color: white; }"
            "QPushButton:pressed { background-color: #a93226; }"
        );
        mainLayout->addWidget(resetBtn);

        // Connections
        connect(memSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &GpuControl::updateEquiv);
        connect(powerSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &GpuControl::updatePowerRatio);
        connect(defaultBtn, &QPushButton::clicked, [this]() { setPreset(defaultPowerLimit, 0, 0); });
        connect(lowBtn, &QPushButton::clicked, [this]() { setPreset((int)(defaultPowerLimit * 0.75), 0, 0); });
        connect(fullBtn, &QPushButton::clicked, [this]() { setPreset(maxPowerLimit, 0, 0); });
        connect(applyBtn, &QPushButton::clicked, this, &GpuControl::applySettings);
        connect(resetBtn, &QPushButton::clicked, this, &GpuControl::resetDefaults);

        loadConfig();
        ensureSudoAccess();
        readCurrentValues();
    }

private slots:
    void updateEquiv() {
        int afterburner = memSpin->value() / 2;
        memEquiv->setText(QString("+%1").arg(afterburner));
    }

    void updatePowerRatio() {
        int current = powerSpin->value();
        int pct = (current * 100) / maxPowerLimit;
        powerRatioLabel->setText(QString("%1 / %2 W  (%3%)").arg(current).arg(maxPowerLimit).arg(pct));
    }

    void setPreset(int power, int mem, int core) {
        powerSpin->setValue(power);
        memSpin->setValue(mem);
        coreSpin->setValue(core);
    }

    void ensureSudoAccess() {
        QProcess check;
        check.start("sudo", QStringList() << "-n" << "nvidia-smi" << "-L");
        check.waitForFinished(3000);

        if (check.exitCode() == 0)
            return;

        QString user = qEnvironmentVariable("USER");
        if (user.isEmpty()) {
            QProcess whoami;
            whoami.start("whoami");
            whoami.waitForFinished(2000);
            user = whoami.readAllStandardOutput().trimmed();
        }

        QString rule = user + " ALL=(ALL) NOPASSWD: /usr/bin/nvidia-smi, /usr/bin/nvidia-settings, /usr/bin/bash, /usr/bin/systemctl";

        QMessageBox::information(this, "First Run Setup",
            "GPU Control needs one-time permission to manage your GPU.\n"
            "You'll be asked for your password once.");

        QProcess setup;
        setup.start("pkexec", QStringList() << "bash" << "-c"
            << QString("echo '%1' > /etc/sudoers.d/gpu-control && chmod 440 /etc/sudoers.d/gpu-control").arg(rule));
        setup.waitForFinished(30000);

        if (setup.exitCode() != 0) {
            QMessageBox::warning(this, "Setup Failed",
                "Could not set up permissions. The app will still work but may ask for your password.");
        }
    }

    void resetDefaults() {
        setPreset(defaultPowerLimit, 0, 0);
        applySettings();
        statusLabel->setText("Reset to stock defaults");
        statusLabel->setStyleSheet("color: #e67e22; padding: 6px; font-size: 13px; font-weight: bold;");
    }

    void applySettings() {
        int power = powerSpin->value();
        int mem = memSpin->value();
        int core = coreSpin->value();
        bool startup = startupCheck->isChecked();

        QStringList errors;

        QProcess plProc;
        plProc.start("sudo", QStringList() << "nvidia-smi" << "-pl" << QString::number(power));
        plProc.waitForFinished(5000);
        if (plProc.exitCode() != 0)
            errors << "Power limit: " + plProc.readAllStandardError().trimmed();

        QProcess memProc;
        memProc.start("sudo", QStringList() << "nvidia-settings" << "-a"
            << QString("[gpu:0]/GPUMemoryTransferRateOffsetAllPerformanceLevels=%1").arg(mem));
        memProc.waitForFinished(5000);
        if (memProc.exitCode() != 0)
            errors << "Memory offset: " + memProc.readAllStandardError().trimmed();

        QProcess coreProc;
        coreProc.start("sudo", QStringList() << "nvidia-settings" << "-a"
            << QString("[gpu:0]/GPUGraphicsClockOffsetAllPerformanceLevels=%1").arg(core));
        coreProc.waitForFinished(5000);
        if (coreProc.exitCode() != 0)
            errors << "Core offset: " + coreProc.readAllStandardError().trimmed();

        if (startup) {
            writeStartupService(power, mem, core);
        } else {
            removeStartupService();
        }

        saveConfig(power, mem, core, startup);

        if (errors.isEmpty()) {
            statusLabel->setText(QString("Applied: %1W | Mem +%2 | Core +%3").arg(power).arg(mem).arg(core));
            statusLabel->setStyleSheet("color: #4CAF50; padding: 6px; font-size: 13px; font-weight: bold;");
        } else {
            QMessageBox::warning(this, "Errors", errors.join("\n"));
        }
    }

private:
    QSpinBox *powerSpin;
    QSpinBox *memSpin;
    QSpinBox *coreSpin;
    QLabel *memEquiv;
    QLabel *statusLabel;
    QLabel *gpuNameLabel;
    QLabel *powerRatioLabel;
    QCheckBox *startupCheck;
    int maxPowerLimit;
    int defaultPowerLimit;

    int queryMaxPowerLimit() {
        QProcess proc;
        proc.start("nvidia-smi", QStringList()
            << "--query-gpu=power.max_limit"
            << "--format=csv,noheader,nounits");
        proc.waitForFinished(5000);
        QString output = proc.readAllStandardOutput().trimmed();
        int val = output.split(".").first().toInt();
        return val > 0 ? val : 450;
    }

    int queryDefaultPowerLimit() {
        QProcess proc;
        proc.start("nvidia-smi", QStringList()
            << "--query-gpu=power.default_limit"
            << "--format=csv,noheader,nounits");
        proc.waitForFinished(5000);
        QString output = proc.readAllStandardOutput().trimmed();
        int val = output.split(".").first().toInt();
        return val > 0 ? val : 450;
    }

    QString queryGpuName() {
        QProcess proc;
        proc.start("nvidia-smi", QStringList()
            << "--query-gpu=name"
            << "--format=csv,noheader");
        proc.waitForFinished(5000);
        QString name = proc.readAllStandardOutput().trimmed();
        return name.isEmpty() ? "NVIDIA GPU" : name;
    }

    QString configPath() {
        return QDir::homePath() + "/.config/gpu-control.conf";
    }

    void saveConfig(int power, int mem, int core, bool startup) {
        QFile file(configPath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "power=" << power << "\n";
            out << "memory=" << mem << "\n";
            out << "core=" << core << "\n";
            out << "startup=" << (startup ? "1" : "0") << "\n";
        }
    }

    void loadConfig() {
        QFile file(configPath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                auto parts = line.split("=");
                if (parts.size() == 2) {
                    if (parts[0] == "power") powerSpin->setValue(parts[1].toInt());
                    if (parts[0] == "memory") memSpin->setValue(parts[1].toInt());
                    if (parts[0] == "core") coreSpin->setValue(parts[1].toInt());
                    if (parts[0] == "startup") startupCheck->setChecked(parts[1] == "1");
                }
            }
        }
        updateEquiv();
        updatePowerRatio();
    }

    void readCurrentValues() {
        QProcess smiProc;
        smiProc.start("nvidia-smi", QStringList()
            << "--query-gpu=power.limit,clocks.current.memory,clocks.current.graphics"
            << "--format=csv,noheader,nounits");
        smiProc.waitForFinished(5000);
        QString smiOut = smiProc.readAllStandardOutput().trimmed();

        // Get clock offsets from nvidia-settings
        QProcess memOffProc;
        memOffProc.start("nvidia-settings", QStringList() << "-t" << "-q"
            << "[gpu:0]/GPUMemoryTransferRateOffsetAllPerformanceLevels");
        memOffProc.waitForFinished(3000);
        QString memOff = memOffProc.readAllStandardOutput().trimmed();

        QProcess coreOffProc;
        coreOffProc.start("nvidia-settings", QStringList() << "-t" << "-q"
            << "[gpu:0]/GPUGraphicsClockOffsetAllPerformanceLevels");
        coreOffProc.waitForFinished(3000);
        QString coreOff = coreOffProc.readAllStandardOutput().trimmed();

        if (!smiOut.isEmpty()) {
            QStringList parts = smiOut.split(",");
            QString power = parts.size() > 0 ? parts[0].trimmed().split(".").first() + "W" : "?";
            QString status = QString("Current: %1 | Mem +%2 | Core +%3")
                .arg(power)
                .arg(memOff.isEmpty() ? "0" : memOff)
                .arg(coreOff.isEmpty() ? "0" : coreOff);
            statusLabel->setText(status);
        }
    }

    void writeStartupService(int power, int mem, int core) {
        QString setupPath = QDir::homePath() + "/.local/bin/gpu-control-setup.sh";
        QDir().mkpath(QDir::homePath() + "/.local/bin");

        QFile setup(setupPath);
        if (setup.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&setup);
            out << "#!/bin/bash\n";
            out << "cat > /etc/systemd/system/gpu-control.service << 'SERVICEEOF'\n";
            out << "[Unit]\n";
            out << "Description=GPU Control - Power and Clock Offsets\n";
            out << "After=nvidia-persistenced.service display-manager.service\n";
            out << "Wants=display-manager.service\n\n";
            out << "[Service]\n";
            out << "Type=oneshot\n";
            out << "RemainAfterExit=yes\n";
            out << "ExecStart=/usr/bin/nvidia-smi -pl " << power << "\n";
            out << "ExecStartPost=/bin/sleep 3\n";
            out << "ExecStartPost=/usr/bin/nvidia-settings -a [gpu:0]/GPUMemoryTransferRateOffsetAllPerformanceLevels=" << mem << "\n";
            out << "ExecStartPost=/usr/bin/nvidia-settings -a [gpu:0]/GPUGraphicsClockOffsetAllPerformanceLevels=" << core << "\n\n";
            out << "[Install]\n";
            out << "WantedBy=multi-user.target\n";
            out << "SERVICEEOF\n";
            out << "systemctl daemon-reload\n";
            out << "systemctl enable gpu-control.service\n";
            setup.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                 QFile::ReadGroup | QFile::ExeGroup |
                                 QFile::ReadOther | QFile::ExeOther);
        }

        QProcess proc;
        proc.start("sudo", QStringList() << "bash" << setupPath);
        proc.waitForFinished(15000);
    }

    void removeStartupService() {
        QProcess proc;
        proc.start("sudo", QStringList() << "bash" << "-c"
            << "systemctl disable gpu-control.service; rm -f /etc/systemd/system/gpu-control.service; systemctl daemon-reload");
        proc.waitForFinished(15000);
    }
};

#include "gpu-control.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    GpuControl window;
    window.show();
    return app.exec();
}
