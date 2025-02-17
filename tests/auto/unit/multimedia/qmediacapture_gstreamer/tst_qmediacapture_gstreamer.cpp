// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimedia/private/qplatformmediacapture_p.h>
#include <QtQGstreamerMediaPlugin/private/qgstpipeline_p.h>

#include <memory>

QT_USE_NAMESPACE

class tst_QMediaCaptureGStreamer : public QObject
{
    Q_OBJECT

public:
    tst_QMediaCaptureGStreamer();

public slots:
    void init();
    void cleanup();

private slots:
    void constructor_preparesGstPipeline();

private:
    std::unique_ptr<QMediaCaptureSession> session;

    GstPipeline *getGstPipeline()
    {
        return reinterpret_cast<GstPipeline *>(
                QPlatformMediaCaptureSession::nativePipeline(session.get()));
    }

    void dumpGraph(const char *fileNamePrefix)
    {
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(getGstPipeline()),
                                  GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_VERBOSE),
                                  fileNamePrefix);
    }
};

tst_QMediaCaptureGStreamer::tst_QMediaCaptureGStreamer()
{
    qputenv("QT_MEDIA_BACKEND", "gstreamer");
}

void tst_QMediaCaptureGStreamer::init()
{
    session = std::make_unique<QMediaCaptureSession>();
}

void tst_QMediaCaptureGStreamer::cleanup()
{
    session.reset();
}

void tst_QMediaCaptureGStreamer::constructor_preparesGstPipeline()
{
    auto *rawPipeline = getGstPipeline();
    QVERIFY(rawPipeline);

    QGstPipeline pipeline{
        rawPipeline,
        QGstPipeline::NeedsRef,
    };
    QVERIFY(pipeline);

    dumpGraph("constructor_preparesGstPipeline");
}

QTEST_GUILESS_MAIN(tst_QMediaCaptureGStreamer)

#include "tst_qmediacapture_gstreamer.moc"
