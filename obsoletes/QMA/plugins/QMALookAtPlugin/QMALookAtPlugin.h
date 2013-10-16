/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef QMALOOKATPLUGIN_H
#define QMALOOKATPLUGIN_H

#include "QMAPlugin.h"

#include <MMDAI/BoneController.h>
#include <MMDAI/SceneController.h>

class QMALookAtPlugin : public QMAPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMAPlugin)

public:
    QMALookAtPlugin();
    ~QMALookAtPlugin();

public slots:
    void load(MMDAI::SceneController *controller, const QString &baseName);
    void unload();
    void receiveCommand(const QString &command, const QList<QVariant> &arguments);
    void receiveEvent(const QString &type, const QList<QVariant> &arguments);

signals:
    void commandPost(const QString &command, const QList<QVariant> &arguments);
    void eventPost(const QString &type, const QList<QVariant> &arguments);

private:
    bool m_enable;
    int m_maxModel;
    MMDAI::SceneController *m_controller;
    MMDAI::BoneController *m_neckController;
    MMDAI::BoneController *m_eyeController;

    Q_DISABLE_COPY(QMALookAtPlugin)
};

#endif // QMALOOKATPLUGIN_H