// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtTest/QtTest>

#include <private/qmemoryvideobuffer_p.h>
#include <private/qimagevideobuffer_p.h>

using BufferPtr = std::shared_ptr<QAbstractVideoBuffer>;
using MapModes = std::vector<QVideoFrame::MapMode>;

static const MapModes validMapModes = { QVideoFrame::ReadOnly, QVideoFrame::WriteOnly, QVideoFrame::ReadWrite };

class tst_QVideoBuffers : public QObject
{
    Q_OBJECT

public:
    tst_QVideoBuffers() {}
public slots:
    void initTestCase();

private slots:
    void map_changesMappedStateAndReturnsProperMappings_whenBufferIsNotMapped_data();
    void map_changesMappedStateAndReturnsProperMappings_whenBufferIsNotMapped();

    void mapWithNotMappedMode_doesNothing_data();
    void mapWithNotMappedMode_doesNothing();

    void map_doesNothing_whenBufferIsMapped_data();
    void map_doesNothing_whenBufferIsMapped();

    void mapMemoryBufferWithReadOnly_doesntDetachArray();

    void mapMemoryBufferWithWriteModes_detachsArray_data();
    void mapMemoryBufferWithWriteModes_detachsArray();

    void underlyingByteArray_returnsCorrectValueForPlanes();

    void unmap_resetsMappedState_whenBufferIsMapped_data();
    void unmap_resetsMappedState_whenBufferIsMapped();

private:
    QString mapModeToString(QVideoFrame::MapMode mapMode) const
    {
        switch (mapMode) {
            case QVideoFrame::NotMapped:
                return QLatin1String("NotMapped");
            case QVideoFrame::ReadOnly:
                return QLatin1String("ReadOnly");
            case QVideoFrame::WriteOnly:
                return QLatin1String("WriteOnly");
            case QVideoFrame::ReadWrite:
                return QLatin1String("ReadWrite");
            default:
                return QLatin1String("Unknown");
        }
    }

    void generateImageAndMemoryBuffersWithAllModes(const MapModes& modes = validMapModes) const
    {
        QTest::addColumn<BufferPtr>("buffer");
        QTest::addColumn<QVideoFrame::MapMode>("mapMode");

        for (auto mode : modes) {
            QTest::newRow(QString("ImageBuffer, %1").arg(mapModeToString(mode)).toUtf8().constData())
                    << createImageBuffer() << mode;
            QTest::newRow(QString("MemoryBuffer, %1").arg(mapModeToString(mode)).toUtf8().constData())
                    << createMemoryBuffer() << mode;
        }
    }

    BufferPtr createImageBuffer() const
    {
        return std::make_shared<QImageVideoBuffer>(m_image);
    }

    BufferPtr createMemoryBuffer() const
    {
        return std::make_shared<QMemoryVideoBuffer>(m_byteArray, m_byteArray.size() / m_image.height());
    }

    QImage m_image = { QSize(5, 4), QImage::Format_RGBA8888 };
    QByteArray m_byteArray;
};


void tst_QVideoBuffers::initTestCase()
{
    m_image.fill(Qt::gray);
    m_image.setPixelColor(0, 0, Qt::green);
    m_image.setPixelColor(m_image.width() - 1, 0, Qt::blue);
    m_image.setPixelColor(0, m_image.height() - 1, Qt::red);

    m_byteArray.assign(m_image.constBits(), m_image.constBits() + m_image.sizeInBytes());
}

void tst_QVideoBuffers::map_changesMappedStateAndReturnsProperMappings_whenBufferIsNotMapped_data()
{
    generateImageAndMemoryBuffersWithAllModes();
}

void tst_QVideoBuffers::map_changesMappedStateAndReturnsProperMappings_whenBufferIsNotMapped()
{
    QFETCH(BufferPtr, buffer);
    QFETCH(QVideoFrame::MapMode, mapMode);

    auto mappedData = buffer->map(mapMode);

    QCOMPARE(buffer->mapMode(), mapMode);

    QCOMPARE(mappedData.nPlanes, 1);
    QVERIFY(mappedData.data[0]);
    QCOMPARE(mappedData.size[0], 80);
    QCOMPARE(mappedData.bytesPerLine[0], 20);

    const auto data = reinterpret_cast<const char*>(mappedData.data[0]);
    QCOMPARE(QByteArray(data, mappedData.size[0]), m_byteArray);
}

void tst_QVideoBuffers::mapWithNotMappedMode_doesNothing_data()
{
    generateImageAndMemoryBuffersWithAllModes();
}

void tst_QVideoBuffers::mapWithNotMappedMode_doesNothing()
{
    QFETCH(BufferPtr, buffer);
    QFETCH(QVideoFrame::MapMode, mapMode);

    buffer->map(mapMode);

    buffer->map(QVideoFrame::NotMapped);

    QCOMPARE(buffer->mapMode(), mapMode);
}

void tst_QVideoBuffers::map_doesNothing_whenBufferIsMapped_data()
{
    generateImageAndMemoryBuffersWithAllModes();
}

void tst_QVideoBuffers::map_doesNothing_whenBufferIsMapped()
{
    QFETCH(BufferPtr, buffer);
    QFETCH(QVideoFrame::MapMode, mapMode);

    buffer->map(mapMode);
    auto mappedData = buffer->map(QVideoFrame::ReadOnly);
    QCOMPARE(mappedData.nPlanes, 0);
    QCOMPARE(buffer->mapMode(), mapMode);
}

void tst_QVideoBuffers::mapMemoryBufferWithReadOnly_doesntDetachArray()
{
    auto buffer = createMemoryBuffer();
    auto underlyingArray = buffer->underlyingByteArray(0);

    auto mappedData = buffer->map(QVideoFrame::ReadOnly);
    QCOMPARE(mappedData.nPlanes, 1);
    QCOMPARE(mappedData.data[0], reinterpret_cast<const uchar *>(underlyingArray.constData()));
    QCOMPARE(mappedData.data[0], reinterpret_cast<const uchar *>(m_byteArray.constData()));
}

void tst_QVideoBuffers::mapMemoryBufferWithWriteModes_detachsArray_data()
{
    QTest::addColumn<QVideoFrame::MapMode>("mapMode");

    QTest::newRow(mapModeToString(QVideoFrame::WriteOnly).toUtf8().constData()) << QVideoFrame::WriteOnly;
    QTest::newRow(mapModeToString(QVideoFrame::WriteOnly).toUtf8().constData()) << QVideoFrame::WriteOnly;
}

void tst_QVideoBuffers::mapMemoryBufferWithWriteModes_detachsArray()
{
    QFETCH(QVideoFrame::MapMode, mapMode);

    auto buffer = createMemoryBuffer();
    auto underlyingArray = buffer->underlyingByteArray(0);

    auto mappedData = buffer->map(mapMode);
    QCOMPARE(mappedData.nPlanes, 1);
    QCOMPARE_NE(mappedData.data[0], reinterpret_cast<const uchar *>(underlyingArray.constData()));
}

void tst_QVideoBuffers::underlyingByteArray_returnsCorrectValueForPlanes()
{
    auto buffer = createMemoryBuffer();

    QCOMPARE(buffer->underlyingByteArray(0).constData(), m_byteArray.constData());

    QVERIFY(buffer->underlyingByteArray(-1).isNull());
    QVERIFY(buffer->underlyingByteArray(1).isNull());
    QVERIFY(buffer->underlyingByteArray(2).isNull());
}

void tst_QVideoBuffers::unmap_resetsMappedState_whenBufferIsMapped_data()
{
    generateImageAndMemoryBuffersWithAllModes();
}

void tst_QVideoBuffers::unmap_resetsMappedState_whenBufferIsMapped()
{
    QFETCH(BufferPtr, buffer);
    QFETCH(QVideoFrame::MapMode, mapMode);

    buffer->map(mapMode);

    buffer->unmap();

    QCOMPARE(buffer->mapMode(), QVideoFrame::NotMapped);

    // Check buffer is valid and it's possible to map again
    auto mappedData = buffer->map(QVideoFrame::ReadOnly);
    QCOMPARE(mappedData.nPlanes, 1);
    QCOMPARE(buffer->mapMode(), QVideoFrame::ReadOnly);

    const auto data = reinterpret_cast<const char*>(mappedData.data[0]);
    QCOMPARE(QByteArray(data, mappedData.size[0]), m_byteArray);
}

QTEST_APPLESS_MAIN(tst_QVideoBuffers);

#include "tst_qvideobuffers.moc"
