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

#ifndef CGISLISTDB_H
#define CGISLISTDB_H

#include <QTreeWidget>
#include <QSqlDatabase>

#include <gis/db/IDBFolder.h>

struct action_t;
class QMenu;
class CDBFolderDatabase;

class CGisListDB : public QTreeWidget
{
    Q_OBJECT
    public:
        CGisListDB(QWidget * parent);
        virtual ~CGisListDB();

        bool event(QEvent * e);

    private slots:
        void slotContextMenu(const QPoint& point);
        void slotAddFolder();
        void slotDelFolder();
        void slotDelLostFound();
        void slotItemExpanded(QTreeWidgetItem * item);
        void slotItemChanged(QTreeWidgetItem * item, int column);

    private:
        friend class CGisListDBEditLock;

        CDBFolderDatabase *getDataBase(const QString& name);
        void addFolder(IDBFolder::type_e type, quint64 key, IDBFolder *parent);

        int isInternalEdit;

        QMenu * menuFolder;
        QAction * actionAddFolder;
        QAction * actionDelFolder;

        QMenu * menuDatabase;

        QMenu * menuLostFound;
        QAction * actionDelLostFound;

        CDBFolderDatabase *  folderDatabase;
};

#endif //CGISLISTDB_H

