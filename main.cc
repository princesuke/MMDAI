/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#include <QtGui/QApplication>
#include "QMAWindow.h"

#ifdef QMA_BUNDLE_PLUGINS
Q_IMPORT_PLUGIN(qma_audio_plugin);
Q_IMPORT_PLUGIN(qma_julius_plugin);
Q_IMPORT_PLUGIN(qma_lookat_plugin);
Q_IMPORT_PLUGIN(qma_openjtalk_plugin);
Q_IMPORT_PLUGIN(qma_vimanager_plugin);
#endif

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QMAWindow window;
  QTranslator appTranslator, qtTranslator;
  const QString locale = QLocale::system().name();

  qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QDir appDir = QDir(app.applicationDirPath());
#if defined(Q_OS_WIN)
  QString dirName = appDir.dirName().toLower();
  if (dirName == "debug" || dirName == "release")
    appDir.cdUp();
  app.addLibraryPath(appDir.absoluteFilePath("Plugins"));
#endif
#ifdef Q_OS_MAC
  QString path = QString("%1/../Resources/QMA_%2")
                 .arg(appDir.absolutePath())
                 .arg(locale);
  appTranslator.load(path);
#else
  const QString path = QString("%1/locales/QMA_%2")
                       .arg(appDir.absolutePath())
                       .arg(locale);
  appTranslator.load(path);
#endif
  app.installTranslator(&appTranslator);

  app.setOrganizationDomain("com.github.hkrn.mmdai");
  app.setOrganizationName("MMDAI Project");
  app.setApplicationName("QtMMDAI");
  app.setApplicationVersion("0.4");
  window.show();

  return app.exec();
}
