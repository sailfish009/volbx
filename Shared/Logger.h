#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QLatin1String>
#include <QMap>
#include <QObject>
#include <QWidget>

#include "LogType.h"

#define LOG(type, msg) \
    Logger::getInstance().log(type, __FILE__, __FUNCTION__, __LINE__, msg)

class QTextEdit;
class QWidget;

/**
 * @brief GUI Logger class.
 */
class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger& getInstance();

    void log(LogTypes type, const char* file, const char* function, int line,
             const QString& msg);

    void switchVisibility();

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger() override = default;

    QTextEdit* createLogsTextEdit();

    void reloadCheckBoxes();

    QMap<LogTypes, bool> activeLogs_;

    /// Widget to display logs (text edit on it).
    QWidget display_;

    const QMap<LogTypes, QString> logNames_{
        {LogTypes::DB, "DATA_BASE"},
        {LogTypes::CONFIG, "CONFIG"},
        {LogTypes::MODEL, "DATA_MODEL"},
        {LogTypes::CALC, "CALCULATIONS"},
        {LogTypes::NETWORK, "NETWORK"},
        {LogTypes::LOGIN, "LOGIN"},
        {LogTypes::APP, "APPLICATION"},
        {LogTypes::IMPORT_EXPORT, "IMPORT_EXPORT"}};

private Q_SLOTS:
    void changeActiveLogs(bool state);
};

#endif  // LOGWINDOW_H
