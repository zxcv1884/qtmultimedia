/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qurl.h"

#include <QtCore/qdebug.h>

#include "mfplayercontrol_p.h"
#include "mfevrvideowindowcontrol_p.h"
#include "mfvideorenderercontrol_p.h"
#include "mfaudioprobecontrol_p.h"
#include "mfvideoprobecontrol_p.h"
#include "mfplayerservice_p.h"
#include "mfplayersession_p.h"
#include "mfmetadatacontrol_p.h"

MFPlayerService::MFPlayerService()
{
    m_session = new MFPlayerSession(this);
    m_player = new MFPlayerControl(m_session);
    m_metaDataControl = new MFMetaDataControl(this);
}

MFPlayerService::~MFPlayerService()
{
    m_session->close();

    if (m_videoWindowControl)
        delete m_videoWindowControl;

    if (m_videoRendererControl)
        delete m_videoRendererControl;

    m_session->Release();
}

QObject *MFPlayerService::requestControl(const char *name)
{
    if (qstrcmp(name, QMediaPlayerControl_iid) == 0) {
        return m_player;
    } else if (qstrcmp(name, QMetaDataReaderControl_iid) == 0) {
        return m_metaDataControl;
    } else if (qstrcmp(name, QVideoRendererControl_iid) == 0) {
        if (!m_videoRendererControl && !m_videoWindowControl) {
            m_videoRendererControl = new MFVideoRendererControl;
            return m_videoRendererControl;
        }
    } else if (qstrcmp(name, QVideoWindowControl_iid) == 0) {
        if (!m_videoRendererControl && !m_videoWindowControl) {
            m_videoWindowControl = new MFEvrVideoWindowControl;
            return m_videoWindowControl;
        }
    } else if (qstrcmp(name,QMediaAudioProbeControl_iid) == 0) {
        if (m_session) {
            MFAudioProbeControl *probe = new MFAudioProbeControl(this);
            m_session->addProbe(probe);
            return probe;
        }
        return 0;
    } else if (qstrcmp(name,QMediaVideoProbeControl_iid) == 0) {
        if (m_session) {
            MFVideoProbeControl *probe = new MFVideoProbeControl(this);
            m_session->addProbe(probe);
            return probe;
        }
        return 0;
    }

    return 0;
}

void MFPlayerService::releaseControl(QObject *control)
{
    if (!control) {
        qWarning("QMediaService::releaseControl():"
                " Attempted release of null control");
    } else if (control == m_videoRendererControl) {
        m_videoRendererControl->setSurface(0);
        delete m_videoRendererControl;
        m_videoRendererControl = 0;
        return;
    } else if (control == m_videoWindowControl) {
        delete m_videoWindowControl;
        m_videoWindowControl = 0;
        return;
    }

    MFAudioProbeControl* audioProbe = qobject_cast<MFAudioProbeControl*>(control);
    if (audioProbe) {
        if (m_session)
            m_session->removeProbe(audioProbe);
        delete audioProbe;
        return;
    }

    MFVideoProbeControl* videoProbe = qobject_cast<MFVideoProbeControl*>(control);
    if (videoProbe) {
        if (m_session)
            m_session->removeProbe(videoProbe);
        delete videoProbe;
        return;
    }
}

QMediaPlayerControl *MFPlayerService::player()
{
    return m_player;
}

QMetaDataReaderControl *MFPlayerService::dataReader()
{
    return m_metaDataControl;
}

QMediaVideoProbeControl *MFPlayerService::videoProbe()
{
    if (m_session) {
        MFVideoProbeControl *probe = new MFVideoProbeControl(this);
        m_session->addProbe(probe);
        return probe;
    }
    return 0;
}

void MFPlayerService::releaseVideoProbe(QMediaVideoProbeControl *probe)
{
    MFVideoProbeControl* videoProbe = qobject_cast<MFVideoProbeControl*>(probe);
    if (m_session)
        m_session->removeProbe(videoProbe);
    delete videoProbe;
}

QMediaAudioProbeControl *MFPlayerService::audioProbe()
{
    if (m_session) {
        MFAudioProbeControl *probe = new MFAudioProbeControl(this);
        m_session->addProbe(probe);
        return probe;
    }
}

void MFPlayerService::releaseAudioProbe(QMediaAudioProbeControl *probe)
{
    MFAudioProbeControl* audioProbe = qobject_cast<MFAudioProbeControl*>(probe);
    if (m_session)
        m_session->removeProbe(audioProbe);
    delete audioProbe;
}

QVideoRendererControl *MFPlayerService::createVideoRenderer()
{
    return m_videoRendererControl;
}

QVideoWindowControl *MFPlayerService::createVideoWindow()
{
    return m_videoWindowControl;
}

MFVideoRendererControl* MFPlayerService::videoRendererControl() const
{
    return m_videoRendererControl;
}

MFEvrVideoWindowControl* MFPlayerService::videoWindowControl() const
{
    return m_videoWindowControl;
}

MFMetaDataControl* MFPlayerService::metaDataControl() const
{
    return m_metaDataControl;
}
