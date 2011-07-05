/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@free.fr>        *
 *  All rights reserved.                                                   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program (COPYING.FREEMEDFORMS file).                   *
 *  If not, see <http://www.gnu.org/licenses/>.                            *
 ***************************************************************************/
/***************************************************************************
 *   Main Developpers :                                                    *
 *       Guillaume Denry <guillaume.denry@gmail.com>                       *
 *       Eric MAEKER, MD <eric.maeker@gmail.com>                           *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "agendamode.h"
#include "useragendasviewer.h"
#include "agendabase.h"

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/ipatient.h>
#include <coreplugin/iuser.h>
#include <coreplugin/constants_menus.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/modemanager/modemanager.h>


using namespace Agenda::Internal;

static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }
static inline Core::IPatient *patient() { return Core::ICore::instance()->patient(); }
static inline Core::IUser *user() { return Core::ICore::instance()->user(); }
static inline Core::ModeManager *modeManager() {return Core::ICore::instance()->modeManager();}
static inline Agenda::Internal::AgendaBase *base() {return Agenda::Internal::AgendaBase::instance();}

AgendaMode::AgendaMode(QObject *parent) :
        Core::BaseMode(parent)
{
    setName(tr("Agenda"));
    setIcon(theme()->icon(Core::Constants::ICONAGENDA, Core::ITheme::BigIcon));
    setPriority(Core::Constants::P_MODE_AGENDA);
    setUniqueModeName(Core::Constants::MODE_AGENDA);
//    const QList<int> &context;
//    setContext();
//    m_Stack = new QStackedWidget;
//    setWidget(m_Stack);
    m_Viewer = new UserAgendasViewer;
    setWidget(m_Viewer);

//    connect(Core::ICore::instance(), SIGNAL(coreOpened()), this, SLOT(postCoreInitialization()));
    connect(modeManager(), SIGNAL(currentModeChanged(Core::IMode*)), this, SLOT(modeActivated(Core::IMode*)));
}

void AgendaMode::postCoreInitialization()
{
}

void AgendaMode::modeActivated(Core::IMode *mode)
{
    static bool lastWasMe = false;
    if (mode==this) {
        patient()->hidePatientBar();
        lastWasMe = true;
        connect(user(), SIGNAL(userChanged()), this, SLOT(userChanged()));
        userChanged();
    } else if (lastWasMe) {
        patient()->showPatientBar();
        lastWasMe = false;
        disconnect(user(), SIGNAL(userChanged()), this, SLOT(userChanged()));
    }
}

void AgendaMode::userChanged()
{
    // get its calendars and set to calendarviewer
//    m_Viewer->setUserCalendar(base()->getUserCalendars());
}
