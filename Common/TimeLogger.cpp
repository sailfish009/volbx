#include "TimeLogger.h"

#include "Logger.h"

TimeLogger::TimeLogger(LogTypes logType, QString operationName)
    : logType_(logType), operationName_(std::move(operationName))
{
    performanceTimer_.start();
}

TimeLogger::~TimeLogger()
{
    QString seconds{QString::number(performanceTimer_.elapsed() * 1.0 / 1000)};
    LOG(logType_, operationName_ + " in " + seconds + " seconds.");
}
