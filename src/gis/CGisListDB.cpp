/**********************************************************************************************
    Copyright (C) 2014 Oliver Eichler oliver.eichler@gmx.de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************/

#include "gis/CGisListDB.h"
#include "gis/CGisWidget.h"
#include "gis/db/macros.h"
#include "gis/db/CDBFolderDatabase.h"
#include "gis/db/CDBFolderLostFound.h"
#include "gis/db/CDBItem.h"
#include "gis/db/CSetupFolder.h"
#include "gis/db/CSetupDatabase.h"
#include "helpers/CSettings.h"
#include "config.h"

#include <QtSql>
#include <QtWidgets>

class CGisListDBEditLock
{
    public:
        CGisListDBEditLock(bool waitCursor, CGisListDB * widget) : widget(widget), waitCursor(waitCursor)
        {
            if(waitCursor)
            {
                QApplication::setOverrideCursor(Qt::WaitCursor);
            }
            widget->isInternalEdit += 1;
        }
        ~CGisListDBEditLock()
        {
            if(waitCursor)
            {
                QApplication::restoreOverrideCursor();
            }
            widget->isInternalEdit -= 1;
        }
    private:
        CGisListDB * widget;
        bool waitCursor;
};


CGisListDB::CGisListDB(QWidget *parent)
    : QTreeWidget(parent)
    , isInternalEdit(0)
{   

    SETTINGS;
    QStringList names = cfg.value("Database/names").toStringList();
    QStringList files = cfg.value("Database/files").toStringList();

    const int N = names.count();
    for(int i = 0; i < N; i++)
    {
        addDatabase(names[i], files[i]);
    }

    menuNone            = new QMenu(this);
    actionAddDatabase   = menuNone->addAction(QIcon("://icons/32x32/Add.png"), tr("Add Database"), this, SLOT(slotAddDatabase()));

    menuFolder          = new QMenu(this);
    actionAddFolder     = menuFolder->addAction(QIcon("://icons/32x32/Add.png"), tr("Add Folder"), this, SLOT(slotAddFolder()));
    actionDelFolder     = menuFolder->addAction(QIcon("://icons/32x32/DeleteOne.png"), tr("Delete Folder"), this, SLOT(slotDelFolder()));

    menuDatabase        = new QMenu(this);
    menuDatabase->addAction(actionAddFolder);
    actionDelDatabase   = menuDatabase->addAction(QIcon("://icons/32x32/DeleteOne.png"), tr("Remove Database"), this, SLOT(slotDelDatabase()));

    menuLostFound       = new QMenu(this);
    actionDelLostFound  = menuLostFound->addAction(QIcon("://icons/32x32/Empty.png"), tr("Empty"), this, SLOT(slotDelLostFound()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(slotItemExpanded(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

CGisListDB::~CGisListDB()
{
    SETTINGS;
    QStringList names;
    QStringList files;

    const int N = topLevelItemCount();
    for(int n = 0; n < N; n++)
    {
        CDBFolderDatabase * database = dynamic_cast<CDBFolderDatabase*>(topLevelItem(n));
        if(database)
        {
            names << database->text(IDBFolder::eColumnName);
            files << database->getFilename();
        }
    }

    cfg.setValue("Database/names", names);
    cfg.setValue("Database/files", files);

}


CDBFolderDatabase * CGisListDB::getDataBase(const QString& name)
{
    CGisListDBEditLock lock(true, this);
    const int N = topLevelItemCount();
    for(int n = 0; n < N; n++)
    {
        CDBFolderDatabase * database = dynamic_cast<CDBFolderDatabase*>(topLevelItem(n));
        if(database && (database->getDBName() == name))
        {
            return database;
        }
    }
    return 0;
}

bool CGisListDB::hasDatabase(const QString& name)
{
    CGisListDBEditLock lock(true, this);
    const int N = topLevelItemCount();
    for(int i = 0; i < N; i++)
    {
        CDBFolderDatabase * folder = dynamic_cast<CDBFolderDatabase*>(topLevelItem(i));
        if(folder && (folder->text(IDBFolder::eColumnName) == name))
        {
            return true;
        }
    }
    return false;
}


bool CGisListDB::event(QEvent * e)
{
    CGisListDBEditLock lock(true, this);

    switch(e->type())
    {
    case eEvtW2DAckInfo:
    {
        CEvtW2DAckInfo * evt = (CEvtW2DAckInfo*)e;
        IDBFolder * folder = getDataBase(evt->db);
        if(folder)
        {
            folder->update(evt);

        }
        e->accept();
        return true;
    }
    }

    return QTreeWidget::event(e);
}

void CGisListDB::slotContextMenu(const QPoint& point)
{
    QPoint p = mapToGlobal(point);

    if(selectedItems().isEmpty())
    {
        menuNone->exec(p);
        return;
    }

    CDBFolderDatabase * database = dynamic_cast<CDBFolderDatabase*>(currentItem());
    if(database)
    {
        menuDatabase->exec(p);
        return;
    }

    CDBFolderLostFound * lostFound = dynamic_cast<CDBFolderLostFound*>(currentItem());
    if(lostFound)
    {
        menuLostFound->exec(p);
        return;
    }

    IDBFolder * folder = dynamic_cast<IDBFolder*>(currentItem());
    if(folder)
    {
        menuFolder->exec(p);
        return;
    }
}

void CGisListDB::slotAddDatabase()
{
    QString name, filename("-");
    CSetupDatabase dlg(name, filename, *this);
    if(dlg.exec() != QDialog::Accepted)
    {
        return ;
    }

    addDatabase(name, filename);
    emit sigChanged();
}

void CGisListDB::addDatabase(const QString& name, const QString& filename)
{
    new CDBFolderDatabase(filename, name, this);

}

void CGisListDB::slotDelDatabase()
{
    CDBFolderDatabase * folder = dynamic_cast<CDBFolderDatabase*>(currentItem());
    if(folder == 0)
    {
        return;
    }

    int res = QMessageBox::question(this, tr("Remove database..."), tr("Do you realy want to remove '%1' from the list?").arg(folder->text(IDBFolder::eColumnName)), QMessageBox::Ok|QMessageBox::Abort, QMessageBox::Ok);
    if(res != QMessageBox::Ok)
    {
        return;
    }

    delete folder;

    emit sigChanged();
}

void CGisListDB::slotAddFolder()
{
    CGisListDBEditLock lock(false, this);

    IDBFolder * folder = dynamic_cast<IDBFolder*>(currentItem());
    if(folder == 0)
    {
        return;
    }

    IDBFolder::type_e type = IDBFolder::eTypeProject;
    QString name;
    CSetupFolder dlg(type, name, this);
    if(dlg.exec() != QDialog::Accepted)
    {
        return;
    }

    folder->addFolder(type, name);
}

void CGisListDB::slotDelFolder()
{
    CGisListDBEditLock lock(false, this);
    IDBFolder * folder = dynamic_cast<IDBFolder*>(currentItem());
    if(folder == 0)
    {
        return;
    }

    int res = QMessageBox::question(this, tr("Delete database folder..."), tr("Are you sure you want to delete \"%1\" from the database?").arg(folder->text(1)), QMessageBox::Ok|QMessageBox::No);
    if(res != QMessageBox::Ok)
    {
        return;
    }

    CDBFolderDatabase * dbfolder = folder->getDBFolder();

    folder->remove();
    delete folder;

    if(dbfolder)
    {
        dbfolder->updateLostFound();
    }
}

void CGisListDB::slotDelLostFound()
{
    CGisListDBEditLock lock(false, this);
    CDBFolderLostFound * folder = dynamic_cast<CDBFolderLostFound*>(currentItem());
    if(folder == 0)
    {
        return;
    }

    int res = QMessageBox::question(this, tr("Remove items..."), tr("Are you sure you want to delete all items from Lost&Found? This will remove them permanently."), QMessageBox::Ok|QMessageBox::No);
    if(res != QMessageBox::Ok)
    {
        return;
    }

    folder->clear();
}

void CGisListDB::slotItemExpanded(QTreeWidgetItem * item)
{   
    CGisListDBEditLock lock(true, this);

    IDBFolder * folder = dynamic_cast<IDBFolder*>(item);
    if(folder == 0)
    {
        return;
    }

    folder->expanding();
}

void CGisListDB::slotItemChanged(QTreeWidgetItem * item, int column)
{
    if(isInternalEdit)
    {
        return;
    }
    CGisListDBEditLock lock(true, this);

    if(column == IDBFolder::eColumnCheckbox)
    {
        IDBFolder * folder = dynamic_cast<IDBFolder*>(item);
        if(folder != 0)
        {
            folder->toggle();
            return;
        }

        CDBItem * dbItem = dynamic_cast<CDBItem*>(item);
        if(dbItem != 0)
        {
            dbItem->toggle();
            return;
        }
    }
}

