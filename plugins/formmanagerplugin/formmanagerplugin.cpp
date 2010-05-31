/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                     **
 *   eric.maeker@free.fr                                                   *
 *   All rights reserved.                                                  *
 *                                                                         *
 *   This program is a free and open source software.                      *
 *   It is released under the terms of the new BSD License.                *
 *                                                                         *
 *   Redistribution and use in source and binary forms, with or without    *
 *   modification, are permitted provided that the following conditions    *
 *   are met:                                                              *
 *   - Redistributions of source code must retain the above copyright      *
 *   notice, this list of conditions and the following disclaimer.         *
 *   - Redistributions in binary form must reproduce the above copyright   *
 *   notice, this list of conditions and the following disclaimer in the   *
 *   documentation and/or other materials provided with the distribution.  *
 *   - Neither the name of the FreeMedForms' organization nor the names of *
 *   its contributors may be used to endorse or promote products derived   *
 *   from this software without specific prior written permission.         *
 *                                                                         *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   *
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     *
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS     *
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE        *
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,  *
 *   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,  *
 *   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER      *
 *   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT    *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN     *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE       *
 *   POSSIBILITY OF SUCH DAMAGE.                                           *
 ***************************************************************************/
#include "formmanagerplugin.h"
#include "formmanagermode.h"
#include "formmanager.h"
#include "formplaceholder.h"
#include "episodebase.h"
#include "episodemodel.h"

#include <utils/log.h>

#include <coreplugin/dialogs/pluginaboutpage.h>
#include <coreplugin/icore.h>
#include <coreplugin/translators.h>

#include <QtCore/QtPlugin>
#include <QWidget>
#include <QDebug>


using namespace Form;
using namespace Internal;

static inline Form::Internal::EpisodeBase *episodeBase() {return Form::Internal::EpisodeBase::instance();}


FormManagerPlugin::FormManagerPlugin() :
        mode(0)
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "creating FormManagerPlugin";
}

FormManagerPlugin::~FormManagerPlugin()
{
    qWarning() << "FormManagerPlugin::~FormManagerPlugin()";
    if (mode) {
        removeObject(mode);
        delete mode;
        mode = 0;
    }
}

bool FormManagerPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "FormManagerPlugin::initialize";
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    // Add Translator to the Application
    Core::ICore::instance()->translators()->addNewTranslator("formmanagerplugin");

    // Initialize patient base
    episodeBase();
    if (!episodeBase()->isInitialized())
        return false;

    return true;
}

void FormManagerPlugin::extensionsInitialized()
{
    if (Utils::Log::warnPluginsCreation())
        qWarning() << "FormManagerPlugin::extensionsInitialized";

    addAutoReleasedObject(new Core::PluginAboutPage(pluginSpec(), this));

    // Add mode
    FormManager::instance();
    mode = new FormManagerMode(this);
    mode->setWidget(FormManager::instance()->formPlaceHolder());
    addObject(mode);
}


Q_EXPORT_PLUGIN(FormManagerPlugin)
