#pragma once

#include <QPointF>

#include <ColumnType.h>
#include <Quantiles.h>

#include "TransactionData.h"

class QSignalSpy;

/**
 * @brief Tests for PlotDataProvider class.
 */
class PlotDataProviderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testRecomputeGroupingIvalidFormat();
    void testRecomputeGroupingData();
    void testRecomputeGroupingDataEmptyCalcData();

    void testRecompute_data();
    void testRecompute();

private:
    void checkRecomputeGroupingDataForColumnType(ColumnType columnType);

    void checkGroupingDataChangedSignal(
        const QSignalSpy& spy, const QVector<QString>& expectedIntervalsNames,
        const QVector<Quantiles>& expectedQuantilesForIntervals,
        const Quantiles& expectedQuantiles);

    static constexpr int NO_SIGNAL{0};
    static constexpr int SIGNAL{1};

    const QVector<TransactionData> calcData_{
        {QDate(2010, 3, 1), QVariant("column1"), 10},
        {QDate(2010, 3, 4), QVariant("column1"), 15},
        {QDate(2010, 3, 6), QVariant("column1"), 12},
        {QDate(2010, 3, 1), QVariant("column2"), 1},
        {QDate(2010, 3, 4), QVariant("column2"), 5},
        {QDate(2010, 3, 6), QVariant("column2"), 2}};

    Quantiles mainQuantiles_;
    Quantiles firstQuantiles_;
    Quantiles secondQuantiles_;

    QVector<QPointF> points_{QPointF(14669, 10), QPointF(14672, 15),
                             QPointF(14674, 12), QPointF(14669, 1),
                             QPointF(14672, 5),  QPointF(14674, 2)};

    QVector<QPointF> regression_{QPointF(14669, 6.44736842105),
                                 QPointF(14674, 8.42105263158)};

    QVector<double> yAxisValues_{10., 15., 12., 1., 5., 2.};
};
