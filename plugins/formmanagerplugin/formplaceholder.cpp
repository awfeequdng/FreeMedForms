/***************************************************************************
 *   FreeMedicalForms                                                      *
 *   (C) 2008-2010 by Eric MAEKER, MD                                      *
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
 ***************************************************************************/
#include "formplaceholder.h"

#include <formmanagerplugin/formmanager.h>
#include <formmanagerplugin/iformitem.h>
#include <formmanagerplugin/iformwidgetfactory.h>
#include <formmanagerplugin/episodemodel.h>

#include <coreplugin/icore.h>
#include <coreplugin/itheme.h>
#include <coreplugin/constants_icons.h>

#include <utils/widgets/minisplitter.h>
#include <utils/log.h>

#include <QTreeView>
#include <QTreeWidgetItem>
#include <QStackedLayout>
#include <QScrollArea>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QMouseEvent>
#include <QPainter>
#include <QEvent>

#include <QDebug>

using namespace Form;

static inline Form::FormManager *formManager() { return Form::FormManager::instance(); }
static inline Core::ITheme *theme()  { return Core::ICore::instance()->theme(); }


namespace Form {
namespace Internal {
class FormPlaceHolderPrivate
{
public:
    FormPlaceHolderPrivate(FormPlaceHolder *parent) :
            m_EpisodeModel(0),
            m_FileTree(0),
            m_Delegate(0),
            m_EpisodesTable(0),
            m_Stack(0),
            m_GeneralLayout(0),
            m_ButtonLayout(0),
            m_Scroll(0),
            m_New(0),
            m_Remove(0),
            q(parent)
    {
    }

    ~FormPlaceHolderPrivate()
    {
        delete m_FileTree; m_FileTree = 0;
        delete m_Stack; m_Stack = 0;
        delete m_GeneralLayout; m_GeneralLayout=0;
    }

    void populateStackLayout()
    {
        Q_ASSERT(m_Stack);
        if (!m_Stack)
            return;
        clearStackLayout();
        foreach(FormMain *form, formManager()->forms()) {
            if (form->formWidget()) {
                QWidget *w = new QWidget();
                QVBoxLayout *vl = new QVBoxLayout(w);
                vl->setSpacing(0);
                vl->setMargin(0);
                vl->addWidget(form->formWidget());
                int id = m_Stack->addWidget(w);
                m_StackId_FormUuid.insert(id, form->uuid());
            }
        }
    }

private:
    void clearStackLayout()
    {
        /** \todo check leaks */
        for(int i = m_Stack->count(); i>0; --i) {
            QWidget *w = m_Stack->widget(i);
            m_Stack->removeWidget(w);
            delete w;
        }
    }

public:
    EpisodeModel *m_EpisodeModel;
    QTreeView *m_FileTree;
    FormItemDelegate *m_Delegate;
    QTableView *m_EpisodesTable;
    QStackedLayout *m_Stack;
    QGridLayout *m_GeneralLayout;
    QGridLayout *m_ButtonLayout;
    QScrollArea *m_Scroll;
    QHash<int, QString> m_StackId_FormUuid;
    QPushButton *m_New, *m_Remove;

private:
    FormPlaceHolder *q;
};

FormItemDelegate::FormItemDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{
}

void FormItemDelegate::setEpisodeModel(EpisodeModel *model)
{
    m_EpisodeModel = model;
}

void FormItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    if (option.state & QStyle::State_MouseOver) {
        if ((QApplication::mouseButtons() & Qt::LeftButton) == 0)
            pressedIndex = QModelIndex();
        QBrush brush = option.palette.alternateBase();
        if (index == pressedIndex)
            brush = option.palette.dark();
        painter->fillRect(option.rect, brush);
    }

    QStyledItemDelegate::paint(painter, option, index);

    if (index.column()==EpisodeModel::EmptyColumn1 && option.state & QStyle::State_MouseOver) {
        QIcon icon;
        if (option.state & QStyle::State_Selected) {
            if (m_EpisodeModel->isEpisode(index))
                icon = theme()->icon(Core::Constants::ICONVALIDATELIGHT);
            else
                icon = theme()->icon(Core::Constants::ICONADDLIGHT);
        } else {
            if (m_EpisodeModel->isEpisode(index))
                icon = theme()->icon(Core::Constants::ICONVALIDATEDARK);
            else
                icon = theme()->icon(Core::Constants::ICONADDDARK);
        }

        QRect iconRect(option.rect.right() - option.rect.height(),
                       option.rect.top(),
                       option.rect.height(),
                       option.rect.height());

        icon.paint(painter, iconRect, Qt::AlignRight | Qt::AlignVCenter);
    }
}

}
}

FormPlaceHolder::FormPlaceHolder(QWidget *parent) :
        QWidget(parent), d(new Internal::FormPlaceHolderPrivate(this))
{
    // create general layout
    d->m_GeneralLayout = new QGridLayout(this);
    d->m_GeneralLayout->setObjectName("FormPlaceHolder::GeneralLayout");
    d->m_GeneralLayout->setMargin(0);
    d->m_GeneralLayout->setSpacing(0);
    setLayout(d->m_GeneralLayout);

    // create the tree view
    QWidget *w = new QWidget(this);
    d->m_FileTree = new QTreeView(this);
    d->m_FileTree->setObjectName("FormTree");
    d->m_FileTree->setIndentation(10);
    d->m_FileTree->viewport()->setAttribute(Qt::WA_Hover);
    d->m_FileTree->setItemDelegate((d->m_Delegate = new Internal::FormItemDelegate(this)));
    d->m_FileTree->setFrameStyle(QFrame::NoFrame);
    d->m_FileTree->setAttribute(Qt::WA_MacShowFocusRect, false);
//    d->m_FileTree->setStyleSheet("QTreeView#FormTree{background:#dee4ea}");

    connect(d->m_FileTree, SIGNAL(clicked(QModelIndex)), this, SLOT(handleClicked(QModelIndex)));
    connect(d->m_FileTree, SIGNAL(pressed(QModelIndex)), this, SLOT(handlePressed(QModelIndex)));
    connect(d->m_FileTree, SIGNAL(activated(QModelIndex)), this, SLOT(setCurrentEpisode(QModelIndex)));

//    connect(m_ui.editorList, SIGNAL(customContextMenuRequested(QPoint)),
//            this, SLOT(contextMenuRequested(QPoint)));

    // create the central view
    d->m_Scroll = new QScrollArea(this);
    d->m_Scroll->setWidgetResizable(true);
    d->m_Stack = new QStackedLayout(w);
    d->m_Stack->setObjectName("FormPlaceHolder::StackedLayout");

    // create the buttons
//    QWidget *wb = new QWidget(this);
//    d->m_ButtonLayout = new QGridLayout(wb);
//    d->m_ButtonLayout->setSpacing(0);
//    d->m_ButtonLayout->setMargin(0);
//    d->m_New = new QPushButton(theme()->icon(Core::Constants::ICONADD), "", wb);
//    d->m_Remove = new QPushButton(theme()->icon(Core::Constants::ICONREMOVE), "", wb);
//    d->m_ButtonLayout->addWidget(d->m_New, 1, 0);
//    d->m_ButtonLayout->addWidget(d->m_Remove, 1, 1);
//    d->m_ButtonLayout->addWidget(d->m_FileTree, 10, 0, 2, 2);
//    connect(d->m_New, SIGNAL(clicked()), this, SLOT(newEpisode()));
//    connect(d->m_Remove, SIGNAL(clicked()), this, SLOT(removeEpisode()));

    // create splitters
    Utils::MiniSplitter *horiz = new Utils::MiniSplitter(this);
    horiz->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    horiz->setObjectName("FormPlaceHolder::MiniSplitter1");
    horiz->setOrientation(Qt::Horizontal);

    Utils::MiniSplitter *vertic = new Utils::MiniSplitter(this);
    vertic->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vertic->setObjectName("FormPlaceHolder::MiniSplitter::Vertical");
    vertic->setOrientation(Qt::Vertical);

    d->m_EpisodesTable = new QTableView(this);
    d->m_EpisodesTable->horizontalHeader()->hide();
    d->m_EpisodesTable->verticalHeader()->hide();

    horiz->addWidget(d->m_FileTree);
//    horiz->addWidget(wb);
//    vertic->addWidget(d->m_EpisodesTable);
    vertic->addWidget(d->m_Scroll);
    horiz->addWidget(vertic);

    horiz->setStretchFactor(0, 1);
    horiz->setStretchFactor(1, 3);
//    vertic->setStretchFactor(0, 1);
//    vertic->setStretchFactor(1, 3);

    d->m_Scroll->setWidget(w);

    d->m_GeneralLayout->addWidget(horiz, 100, 0);
}

FormPlaceHolder::~FormPlaceHolder()
{
    if (d) {
        delete d;
        d = 0;
    }
}

void FormPlaceHolder::setEpisodeModel(EpisodeModel *model)
{
    d->m_EpisodeModel = model;
    d->m_Delegate->setEpisodeModel(model);
    d->m_FileTree->setModel(model);
    d->m_FileTree->setSelectionMode(QAbstractItemView::SingleSelection);
    d->m_FileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    for(int i=0; i < Form::EpisodeModel::MaxData; ++i)
        d->m_FileTree->setColumnHidden(i, true);
    d->m_FileTree->setColumnHidden(Form::EpisodeModel::Label, false);
    d->m_FileTree->setColumnHidden(Form::EpisodeModel::EmptyColumn1, false);
    d->m_FileTree->header()->hide();
    d->m_FileTree->header()->setStretchLastSection(false);
    d->m_FileTree->header()->setResizeMode(Form::EpisodeModel::Label, QHeaderView::Stretch);
    d->m_FileTree->header()->setResizeMode(Form::EpisodeModel::EmptyColumn1, QHeaderView::Fixed);
    d->m_FileTree->header()->resizeSection(Form::EpisodeModel::EmptyColumn1, 16);
}

void FormPlaceHolder::handlePressed(const QModelIndex &index)
{
    if (index.column() == EpisodeModel::Label) {
//        d->m_EpisodeModel->activateEpisode(index);
    } else if (index.column() == EpisodeModel::EmptyColumn1) {
        d->m_Delegate->pressedIndex = index;
    }
}

void FormPlaceHolder::handleClicked(const QModelIndex &index)
{
    if (index.column() == EpisodeModel::EmptyColumn1) { // the funky button
        if (d->m_EpisodeModel->isForm(index)) {
            newEpisode();
        } else {
            /** \todo validateEpisode */
//            validateEpisode();
        }

        // work around a bug in itemviews where the delegate wouldn't get the QStyle::State_MouseOver
        QPoint cursorPos = QCursor::pos();
        QWidget *vp = d->m_FileTree->viewport();
        QMouseEvent e(QEvent::MouseMove, vp->mapFromGlobal(cursorPos), cursorPos, Qt::NoButton, 0, 0);
        QCoreApplication::sendEvent(vp, &e);
    }
}

QTreeView *FormPlaceHolder::formTree() const
{
    return d->m_FileTree;
}

QStackedLayout *FormPlaceHolder::formStackLayout() const
{
    return d->m_Stack;
}

void FormPlaceHolder::addTopWidget(QWidget *top)
{
    static int lastInsertedRow = 0;
    d->m_GeneralLayout->addWidget(top, lastInsertedRow, 0);
    ++lastInsertedRow;
}

void FormPlaceHolder::addBottomWidget(QWidget *bottom)
{
    d->m_GeneralLayout->addWidget(bottom, d->m_GeneralLayout->rowCount(), 0, 0, d->m_GeneralLayout->columnCount());
}

void FormPlaceHolder::setCurrentForm(const QString &formUuid)
{
    d->m_Stack->setCurrentIndex(d->m_StackId_FormUuid.key(formUuid));
}

void FormPlaceHolder::setCurrentEpisode(const QModelIndex &index)
{
    const QString &formUuid = d->m_EpisodeModel->index(index.row(), EpisodeModel::FormUuid, index.parent()).data().toString();
    setCurrentForm(formUuid);
    if (d->m_EpisodeModel->isEpisode(index)) {
        d->m_EpisodeModel->activateEpisode(index, formUuid);
    }
}

void FormPlaceHolder::reset()
{
    d->m_FileTree->update();
    d->m_FileTree->expandAll();
    d->populateStackLayout();
}

void FormPlaceHolder::newEpisode()
{
    // get the parent form
    QModelIndex index;
    if (!d->m_FileTree->selectionModel()->hasSelection())
        return;
    index = d->m_FileTree->selectionModel()->selectedIndexes().at(0);
    while (!d->m_EpisodeModel->isForm(index)) {
        index = index.parent();
    }

    // create a new episode the selected form and its children
    if (!d->m_EpisodeModel->insertRow(0, index)) {
        Utils::Log::addError(this, "Unable to create new episode");
        return;
    }
    // activate the newly created main episode
    d->m_FileTree->selectionModel()->clearSelection();
    d->m_FileTree->selectionModel()->setCurrentIndex(d->m_EpisodeModel->index(0,0,index), QItemSelectionModel::Select);
    const QString &formUuid = d->m_EpisodeModel->index(index.row(), Form::EpisodeModel::FormUuid, index.parent()).data().toString();
    setCurrentForm(formUuid);
    d->m_EpisodeModel->activateEpisode(d->m_EpisodeModel->index(0,0,index), formUuid);
}

void FormPlaceHolder::removeEpisode()
{
    /** \todo removeEpisode */
}

void FormPlaceHolder::validateEpisode()
{
    /** \todo validateEpisode */
}
