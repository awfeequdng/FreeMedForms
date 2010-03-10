/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   Copyright (C) 2008-2009 by Eric MAEKER                                *
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
/***************************************************************************
 *   Main Developper : Eric MAEKER, <eric.maeker@free.fr>                  *
 *   Contributors :                                                        *
 *       NAME <MAIL@ADRESS>                                                *
 *       NAME <MAIL@ADRESS>                                                *
 ***************************************************************************/

/**
  \class DosageCreator
  \brief Dialog for dosage creation / edition / modification. A dosage is a standard set of datas that will be used to help
  doctors when prescribing a drug.
  If you want to create a new dosage, you must create a new row onto the model BEFORE.
  If you want to edit or modify a dosage, you must inform the dialog of the row and the CIS of the drug.
  \ingroup freediams drugswidget
*/


#include "mfDosageCreatorDialog.h"

// include drugwidget headers
#include <drugsplugin/constants.h>
#include <drugsplugin/drugswidgetmanager.h>

#include <drugsbaseplugin/drugsdata.h>
#include <drugsbaseplugin/dosagemodel.h>
#include <drugsbaseplugin/drugsmodel.h>
#include <drugsbaseplugin/globaldrugsmodel.h>

#include <utils/log.h>
#include <utils/global.h>
#include <translationutils/constanttranslations.h>

#include <coreplugin/icore.h>
#include <coreplugin/isettings.h>
#include <coreplugin/itheme.h>
#include <coreplugin/dialogs/helpdialog.h>

#include <QMessageBox>
#include <QModelIndex>

using namespace DrugsWidget::Constants;
using namespace DrugsWidget::Internal;
using namespace Trans::ConstantTranslations;

inline static DrugsDB::DrugsModel *drugModel() { return DrugsWidget::DrugsWidgetManager::instance()->currentDrugsModel(); }

namespace DrugsWidget {
namespace Internal {

/**
  \brief Private part of DosageDialog
  \internal
*/
class DosageCreatorDialogPrivate
{
public:
    DosageCreatorDialogPrivate(DosageCreatorDialog *parent) :
            m_DosageModel(0), m_Parent(parent) {}

    /** \brief Check the validity o the dosage. Warn if dosage is not valid */
    bool checkDosageValidity(const int row)
    {
        QStringList list = m_DosageModel->isDosageValid(row);
        if (list.count()) {
            Utils::warningMessageBox(QCoreApplication::translate("DosageCreatorDialog", "Dosage is not valid."),
                                        list.join("br />"),
                                        "", QCoreApplication::translate("DosageCreatorDialog", "Drug Dosage Creator"));
            return false;
        }
        return true;
    }

    /** \brief Save the current dirty rows of the model to the database */
    void saveToModel()
    {
        int row = m_Parent->availableDosagesListView->listView()->currentIndex().row();
        // if Inn is checked --> clear dosage CIS, feel INN + COMPO_DOSAGE
        if (!checkDosageValidity(row))
            return;
        m_DosageModel->database().transaction();
        if (m_DosageModel->submitAll()) {
            if (m_DosageModel->database().commit())
                Utils::Log::addMessage(m_Parent, QCoreApplication::translate("DosageCreatorDialog", "Dosage correctly saved to base"));
            else
                Utils::Log::addError(m_Parent, QCoreApplication::translate("DosageCreatorDialog", "SQL Error : Dosage can not be added to database : %1")
                                .arg(m_DosageModel->lastError().text()));
        } else {
            m_DosageModel->database().rollback();
            QMessageBox::warning(m_Parent, QCoreApplication::translate("DosageCreatorDialog", "Drug Dosage Creator"),
                                 tkTr(Trans::Constants::ERROR_1_FROM_DATABASE_2)
                                 .arg(m_DosageModel->database().lastError().text())
                                 .arg(m_DosageModel->database().connectionName()));
        }
    }

    /** \brief Transforms the "reference dialog" to a prescription */
    void toPrescription()
    {
        int row = m_Parent->availableDosagesListView->listView()->currentIndex().row();
        m_DosageModel->toPrescription(row);
    }

public:
    DrugsDB::Internal::DosageModel *m_DosageModel;
    QString      m_ActualDosageUuid;
private:
    DosageCreatorDialog *m_Parent;
};

}  // End Internal
}  // End DrugsWidget

/**
 \todo when showing dosage, make verification of limits +++  ==> for FMF only
 \todo use a QPersistentModelIndex instead of drugRow, dosageRow
*/
DosageCreatorDialog::DosageCreatorDialog( QWidget *parent, DrugsDB::Internal::DosageModel *dosageModel )
    : QDialog( parent ),
    d(0)
{
    using namespace DrugsDB::Constants;
    // some initializations
    setObjectName( "DosageCreatorDialog" );
    d = new DosageCreatorDialogPrivate(this);
    d->m_DosageModel = dosageModel;

    // Ui initialization
    setupUi(this);
    setWindowTitle( tr( "Drug Dosage Creator" ) + " - " + qApp->applicationName() );

    // Drug informations
    int CIS = dosageModel->drugUID();
    drugNameLabel->setText( drugModel()->drugData(CIS, Drug::Denomination).toString() );
    QString toolTip = drugModel()->drugData(CIS, Interaction::ToolTip ).toString();
    interactionIconLabel->setPixmap( drugModel()->drugData(CIS, Interaction::Icon).value<QIcon>().pixmap(16,16) );
    interactionIconLabel->setToolTip( toolTip );
    toolTip = drugModel()->drugData(CIS, Drug::CompositionString ).toString();
    drugNameLabel->setToolTip( toolTip );
    // Various model intializations
    dosageViewer->setDosageModel(dosageModel);
    availableDosagesListView->setModel(dosageModel);
    availableDosagesListView->setModelColumn(Dosages::Constants::Label);
    availableDosagesListView->setEditTriggers( QListView::NoEditTriggers );
    if (dosageModel->rowCount()==0) {
        dosageModel->insertRow(0);
        dosageViewer->changeCurrentRow(0);
    } else {
        dosageViewer->changeCurrentRow(0);
    }

    // Create connections
    connect(availableDosagesListView->listView(), SIGNAL(activated(QModelIndex)),dosageViewer,SLOT(changeCurrentRow(QModelIndex)));
    QModelIndex idx = dosageModel->index(0,Dosages::Constants::Label);
    availableDosagesListView->setCurrentIndex(idx);
}

/** \brief Destructor */
DosageCreatorDialog::~DosageCreatorDialog()
{
    if (d) delete d;
    d=0;
}

/**
   \brief Validate the dialog
   \todo Check dosage validity before validate the dialog
*/
void DosageCreatorDialog::done(int r)
{
    int row = availableDosagesListView->listView()->currentIndex().row();

    if ( r == QDialog::Rejected ) {
        d->m_DosageModel->revertRow(row);
    }  else {
        DrugsDB::GlobalDrugsModel::updateAvailableDosages();
        dosageViewer->done(r);
        /** \todo check validity of the dosage before submition */
    }
    QDialog::done(r);
}

/** \brief Save the "reference dosage" to the database and reject the dialog (no prescription's done) */
void DosageCreatorDialog::on_saveButton_clicked()
{
    // modify focus for the mapper to commit changes
    saveButton->setFocus();
    dosageViewer->commitToModel();
    d->saveToModel();
    done(QDialog::Rejected);
}

/** \brief Accept the dialog (prescription's done), no changes is done on the database. */
void DosageCreatorDialog::on_prescribeButton_clicked()
{
    // modify focus for the mapper to commit changes
    prescribeButton->setFocus();
    dosageViewer->commitToModel();
    d->toPrescription();
    dosageViewer->done(QDialog::Accepted);
    done(QDialog::Accepted);
}

/** \brief Save the "reference dosage" to the database and prescribe it then accept the dialog ( prescription's done) */
void DosageCreatorDialog::on_saveAndPrescribeButton_clicked()
{
    // modify focus for the mapper to commit changes
    saveAndPrescribeButton->setFocus();
    dosageViewer->commitToModel();
    d->toPrescription();
    d->saveToModel();
    dosageViewer->done(QDialog::Accepted);
    done(QDialog::Accepted);
}

/** \brief Opens a help dialog */
void DosageCreatorDialog::on_helpButton_clicked()
{
    Core::HelpDialog::showPage("prescrire.html");
}

void DosageCreatorDialog::on_testOnlyButton_clicked()
{
    drugModel()->setDrugData(d->m_DosageModel->drugUID(), DrugsDB::Constants::Prescription::OnlyForTest, true);
    dosageViewer->done(QDialog::Accepted);
    done(QDialog::Accepted);
}
