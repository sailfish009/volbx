#include "InnerTests.h"

#include <ExportDsv.h>
#include <Qt5Quazip/quazip.h>
#include <Qt5Quazip/quazipfile.h>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDirIterator>
#include <QDomDocument>
#include <QtTest/QtTest>

#include "Common/Constants.h"
#include "Common/ExportUtilities.h"
#include "Datasets/DatasetInner.h"
#include "ModelsAndViews/FilteringProxyModel.h"
#include "ModelsAndViews/TableModel.h"

#include "Common.h"
#include "DatasetUtilities.h"

void InnerTests::initTestCase()
{
    tempFilename_ = "temp";
    // generateDumpData();
}

void InnerTests::generateDumpData()
{
    QDirIterator dirIt(DatasetUtilities::getDatasetsDir(),
                       QDirIterator::Subdirectories);
    while (dirIt.hasNext())
    {
        dirIt.next();
        QFileInfo fileInfo(dirIt.filePath());
        if (fileInfo.isFile() && (fileInfo.suffix() == "vbx"))
        {
            DatasetInner* dataset = new DatasetInner(fileInfo.baseName());
            dataset->initialize();
            QByteArray definitionDump =
                dataset->definitionToXml(dataset->rowCount());
            Common::saveFile(DatasetUtilities::getDatasetsDir() +
                                 fileInfo.baseName() +
                                 Common::getDefinitionDumpSuffix(),
                             definitionDump);

            QVector<bool> activeColumns(dataset->columnCount(), true);
            dataset->setActiveColumns(activeColumns);
            dataset->loadData();

            TableModel model((std::unique_ptr<Dataset>(dataset)));
            FilteringProxyModel proxyModel;
            proxyModel.setSourceModel(&model);

            QTableView view;
            view.setModel(&proxyModel);

            QString tsvData{Common::getExportedTsv(view)};
            Common::saveFile(DatasetUtilities::getDatasetsDir() +
                                 fileInfo.baseName() +
                                 Common::getDataTsvDumpSuffix(),
                             tsvData);
        }
    }
}

void InnerTests::testDatasets()
{
    QStringList datasets = DatasetUtilities::getListOfAvailableDatasets();
    foreach (QString datasetName, datasets)
    {
        DatasetInner* dataset = new DatasetInner(datasetName);
        QVERIFY(true == dataset->initialize());
        QVERIFY(true == dataset->isValid());

        QVector<bool> activeColumns(dataset->columnCount(), true);
        dataset->setActiveColumns(activeColumns);

        dataset->loadData();

        QVERIFY(true == dataset->isValid());

        TableModel model((std::unique_ptr<Dataset>(dataset)));
        FilteringProxyModel proxyModel;
        proxyModel.setSourceModel(&model);

        QTableView view;
        view.setModel(&proxyModel);

        checkImport(datasetName, dataset, view);

        QString filePath{DatasetUtilities::getDatasetsDir() + tempFilename_ +
                         DatasetUtilities::getDatasetExtension()};
        ExportUtilities::saveDataset(filePath, &view, nullptr);
        checkExport(datasetName);
    }
}

void InnerTests::checkDatasetDefinition(const QString& fileName,
                                        DatasetInner* dataset) const
{
    QString datasetFilePath(DatasetUtilities::getDatasetsDir() + fileName);
    const QString dumpFileName{datasetFilePath +
                               Common::getDefinitionDumpSuffix()};
    QByteArray dumpFromFile{Common::loadFile(dumpFileName).toUtf8()};
    QByteArray dumpFromDataset{dataset->definitionToXml(dataset->rowCount())};
    QVERIFY(Common::xmlsAreEqual(dumpFromFile, dumpFromDataset));
}

void InnerTests::checkDatasetData(const QString& fileName,
                                  const QTableView& view) const
{
    QString actualData{Common::getExportedTsv(view)};

    QString datasetFilePath(DatasetUtilities::getDatasetsDir() + fileName);
    QString compareData =
        Common::loadFile(datasetFilePath + Common::getDataTsvDumpSuffix());

    QCOMPARE(actualData.split('\n'), compareData.split('\n'));
}

void InnerTests::checkImport(const QString& fileName, DatasetInner* dataset,
                             const QTableView& view)
{
    checkDatasetDefinition(fileName, dataset);
    checkDatasetData(fileName, view);
}

void InnerTests::checkExport(QString fileName)
{
    // Open original archive.
    QuaZip zipOriginal(DatasetUtilities::getDatasetsDir() + fileName +
                       DatasetUtilities::getDatasetExtension());
    QVERIFY(true == zipOriginal.open(QuaZip::mdUnzip));

    // Open generated archive.
    QuaZip zipGenerated(DatasetUtilities::getDatasetsDir() + tempFilename_ +
                        DatasetUtilities::getDatasetExtension());
    QVERIFY(true == zipGenerated.open(QuaZip::mdUnzip));

    // Open data files in archives and compare it.
    QuaZipFile zipFileOriginal(&zipOriginal);
    zipOriginal.setCurrentFile(DatasetUtilities::getDatasetDataFilename());
    QVERIFY(true == zipFileOriginal.open(QIODevice::ReadOnly));
    QByteArray originalData = zipFileOriginal.readAll();
    zipFileOriginal.close();

    QuaZipFile zipFileGenerated(&zipGenerated);
    zipGenerated.setCurrentFile(DatasetUtilities::getDatasetDataFilename());
    QVERIFY(true == zipFileGenerated.open(QIODevice::ReadOnly));
    QByteArray generatedData = zipFileGenerated.readAll();
    zipFileGenerated.close();
    QCOMPARE(generatedData, originalData);

    // Open strings files in archives and compare it.
    zipOriginal.setCurrentFile(DatasetUtilities::getDatasetStringsFilename());
    QVERIFY(true == zipFileOriginal.open(QIODevice::ReadOnly));
    originalData = zipFileOriginal.readAll();
    zipFileOriginal.close();

    zipGenerated.setCurrentFile(DatasetUtilities::getDatasetStringsFilename());
    QVERIFY(true == zipFileGenerated.open(QIODevice::ReadOnly));
    generatedData = zipFileGenerated.readAll();
    zipFileGenerated.close();
    QCOMPARE(generatedData, originalData);

    // Open definitions files.
    zipOriginal.setCurrentFile(
        DatasetUtilities::getDatasetDefinitionFilename());
    QVERIFY(true == zipFileOriginal.open(QIODevice::ReadOnly));
    originalData = zipFileOriginal.readAll();
    zipFileOriginal.close();

    zipGenerated.setCurrentFile(
        DatasetUtilities::getDatasetDefinitionFilename());
    QVERIFY(true == zipFileGenerated.open(QIODevice::ReadOnly));
    generatedData = zipFileGenerated.readAll();
    zipFileGenerated.close();
    compareDefinitionFiles(generatedData, originalData);
}

void InnerTests::compareDefinitionFiles(QByteArray& original,
                                        QByteArray& generated)
{
    QDomDocument xmlDocumentOryginal(__FUNCTION__);
    QDomDocument xmlDocumentGenerated(__FUNCTION__);
    QVERIFY(true == xmlDocumentOryginal.setContent(original));
    QVERIFY(true == xmlDocumentGenerated.setContent(generated));

    QDomElement rootOriginal = xmlDocumentOryginal.documentElement();
    QDomElement rootGenerated = xmlDocumentGenerated.documentElement();

    QDomNodeList columnsOriginal =
        rootOriginal.firstChildElement("COLUMNS").childNodes();

    QDomNodeList columnsGenerated =
        rootGenerated.firstChildElement("COLUMNS").childNodes();

    int columnsNumber = columnsOriginal.size();
    QCOMPARE(columnsNumber, columnsGenerated.size());
    for (int i = 0; i < columnsNumber; ++i)
    {
        QDomElement originalElement = columnsOriginal.at(i).toElement();
        QDomElement generatedElement = columnsGenerated.at(i).toElement();

        QCOMPARE(originalElement.attribute("NAME"),
                 generatedElement.attribute("NAME"));
        QCOMPARE(originalElement.attribute("FORMAT"),
                 generatedElement.attribute("FORMAT"));
        QCOMPARE(originalElement.attribute("SPECIAL_TAG"),
                 generatedElement.attribute("SPECIAL_TAG"));
    }

    int originalRowCount = rootOriginal.firstChildElement("ROW_COUNT")
                               .attribute("ROW_COUNT")
                               .toInt();
    int generatredRowCount = rootGenerated.firstChildElement("ROW_COUNT")
                                 .attribute("ROW_COUNT")
                                 .toInt();

    QCOMPARE(originalRowCount, generatredRowCount);
}

void InnerTests::testPartialData()
{
    //    DatasetDefinitionInner* definition =
    //        new DatasetDefinitionInner("test");

    //    QVERIFY(true == definition->isValid());

    //    QVector<bool> activeColumns(definition->columnCount(), false);
    //    activeColumns[0] = true;
    //    activeColumns[1] = true;
    //    activeColumns[4] = true;
    //    activeColumns[5] = true;
    //    activeColumns[8] = true;
    //    activeColumns[13] = true;
    //    definition->setActiveColumns(activeColumns);

    //    DatasetInner* dataset = new DatasetInner(definition);
    //    dataset->init();

    //    QVERIFY(true == dataset->isValid());

    //    TableModel model(dataset);
    //    FilteringProxyModel proxyModel;
    //    proxyModel.setSourceModel(&model);

    //    QTableView view;
    //    view.setModel(&proxyModel);
    //    view.setSelectionMode(QAbstractItemView::MultiSelection);
    //    view.setSelectionBehavior(QAbstractItemView::SelectRows);
    //    view.selectAll();
    //    QItemSelectionModel* selectionModel = view.selectionModel();
    //    selectionModel->setCurrentIndex(model.index(1, 0),
    //                                    QItemSelectionModel::Deselect);
    //    ExportData::saveDataset(tempFilename_, &view);

    //    checkExport("partialTest");
}

void InnerTests::cleanupTestCase()
{
    QFile::remove(DatasetUtilities::getDatasetsDir() + tempFilename_ +
                  DatasetUtilities::getDatasetExtension());
}
