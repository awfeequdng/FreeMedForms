/***************************************************************************
 *  The FreeMedForms project is a set of free, open source medical         *
 *  applications.                                                          *
 *  (C) 2008-2011 by Eric MAEKER, MD (France) <eric.maeker@gmail.com>      *
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
 *   Main Developper : Eric MAEKER, <eric.maeker@gmail.com>                *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/
#include "userfistrunpage.h"
#include "widgets/useridentifier.h"
#include "widgets/usermanager.h"
#include "widgets/userwizard.h"
#include "usermanagerplugin/database/userbase.h"
#include "usermanagerplugin/usermodel.h"

#include <coreplugin/icore.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>
#include <coreplugin/isettings.h>
#include <coreplugin/translators.h>

#include <utils/databaseconnector.h>
#include <utils/log.h>
#include <translationutils/constanttranslations.h>

#include "ui_firstrunusercreationwidget.h"

using namespace UserPlugin;
using namespace Trans::ConstantTranslations;

static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }
static inline UserPlugin::UserModel *userModel() {return UserPlugin::UserModel::instance();}
static inline UserPlugin::Internal::UserBase *userBase() {return UserPlugin::Internal::UserBase::instance();}
static inline Core::ISettings *settings() { return Core::ICore::instance()->settings(); }


UserCreationPage::UserCreationPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::FirstRunUserCreationWidget)
{
    ui->setupUi(this);
    ui->userManagerButton->setIcon(theme()->icon(Core::Constants::ICONUSERMANAGER, Core::ITheme::MediumIcon));
    ui->completeWizButton->setIcon(theme()->icon(Core::Constants::ICONNEWUSER, Core::ITheme::MediumIcon));

//    ui->userManagerButton->setEnabled(false);
//    ui->completeWizButton->setEnabled(false);

    QPixmap pix = theme()->splashScreenPixmap("freemedforms-wizard-users.png");
    setPixmap(QWizard::BackgroundPixmap, pix);
    setPixmap(QWizard::WatermarkPixmap, pix);

    connect(ui->userManagerButton, SIGNAL(clicked()), this, SLOT(userManager()));
    connect(ui->completeWizButton, SIGNAL(clicked()), this, SLOT(userWizard()));
}

UserCreationPage::~UserCreationPage()
{
    delete ui;
}

void UserCreationPage::userManager()
{
    UserManagerDialog dlg(this);
    dlg.initialize();
    dlg.exec();
}

void UserCreationPage::userWizard()
{
    UserWizard wiz;
    wiz.exec();
}

void UserCreationPage::initializePage()
{
    // Create the user database
    userBase()->initialize();

    const Utils::DatabaseConnector &db = settings()->databaseConnector();
    if (db.driver()==Utils::Database::SQLite) {
        // keep language
        QLocale::Language l = QLocale().language();
        if (!userModel()->setCurrentUser(Constants::DEFAULT_USER_CLEARLOGIN, Constants::DEFAULT_USER_CLEARPASSWORD)) {
            LOG_ERROR("Unable to connect has default admin user");
            ui->userManagerButton->setEnabled(false);
        }
        // return to the selected language
        Core::ICore::instance()->translators()->changeLanguage(l);
    }
//    else if (db.driver()==Utils::Database::MySQL) {
//        if (!userModel()->setCurrentUser(Constants::DEFAULT_USER_CLEARLOGIN, Constants::DEFAULT_USER_CLEARPASSWORD)) {
//            LOG_ERROR("Unable to connect has default admin user");
//            ui->userManagerButton->setEnabled(false);
//        }
//    }

    // Set current user into user model
    userModel()->setCurrentUserIsServerManager();

    adjustSize();
}

bool UserCreationPage::validatePage()
{
    /** \todo code here */
    // Are there user created ? no -> can not validate
    // disconnected user database ?
    // disconnect currentUser from userModel
    userModel()->clear();

    // remove login/pass from settings()
    Utils::DatabaseConnector db = settings()->databaseConnector();
    db.setClearLog(Constants::DEFAULT_USER_CLEARLOGIN);
    db.setClearPass(Constants::DEFAULT_USER_CLEARPASSWORD);
    settings()->setDatabaseConnector(db);
    settings()->sync();
    Core::ICore::instance()->databaseServerLoginChanged();
    return true;
}

void UserCreationPage::retranslate()
{
    setTitle(QCoreApplication::translate(Constants::TR_CONTEXT_USERS, Constants::CREATE_USER));
    setSubTitle(tr("You can use the full user manager dialog to create user or create simple users using the user wizard."));
//    setSubTitle(tr("You can create user inside FreeMedForms at the end of the configuration."));
    ui->userManagerButton->setText(tkTr(Trans::Constants::USERMANAGER_TEXT));
    ui->completeWizButton->setText(QCoreApplication::translate(Constants::TR_CONTEXT_USERS, Constants::USER_WIZARD));
}

void UserCreationPage::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslate();
        break;
    default:
        break;
    }
}
