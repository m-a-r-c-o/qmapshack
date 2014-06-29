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

#ifndef CCANVAS_H
#define CCANVAS_H

#include <QWidget>
#include <QPainter>
class CMapDraw;
class CGrid;
class CDem;
class QSettings;
class QPointF;
class IMouse;
class QTimer;
class QMovie;
class QLabel;

inline  void USE_ANTI_ALIASING(QPainter& p, bool useAntiAliasing)
{
    p.setRenderHints(QPainter::TextAntialiasing|QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing, useAntiAliasing);
}

#define PROGRESS_SETUP(lbl, max) \
QProgressDialog progress(lbl, "Abort", 0, max, 0);\
progress.setWindowModality(Qt::WindowModal);\

#define PROGRESS(x, cmd) \
progress.setValue(x); \
if (progress.wasCanceled()) cmd;\


class CCanvas : public QWidget
{
    Q_OBJECT
    public:
        CCanvas(QWidget * parent);
        virtual ~CCanvas();

        void saveConfig(QSettings& cfg);
        void loadConfig(QSettings& cfg);

        void setupGrid();
        void convertGridPos2Str(const QPointF& pos, QString& str);

        void moveMap(const QPointF &delta);
        void displayInfo(const QPoint& px);

        static void drawText(const QString& str, QPainter& p, const QPoint& center, const QColor& color);
        static void drawText(const QString& str, QPainter& p, const QPoint& center, const QColor& color, const QFont& font);
        static void drawText(const QString& str, QPainter& p, const QRect& r, const QColor& color);

    signals:
        void sigMousePosition(const QPointF& pos, qreal ele);

    public slots:
        void slotTriggerCompleteUpdate();

    protected:
        void resizeEvent(QResizeEvent * e);
        void paintEvent(QPaintEvent * e);
        void mousePressEvent(QMouseEvent * e);
        void mouseMoveEvent(QMouseEvent * e);
        void mouseReleaseEvent(QMouseEvent *e);
        void wheelEvent(QWheelEvent * e);
        void enterEvent(QEvent * e);
        void leaveEvent(QEvent * e);
        void keyPressEvent(QKeyEvent * e);


    private slots:
        void slotToolTip();

    private:
        void drawScale(QPainter& p);

        /// set true to initiate a complete redraw of the screen content
        bool needsRedraw;
        /// the map object attached to this canvas
        CMapDraw * map;

        /// the grid attached to this canvas
        CGrid * grid;

        CDem * dem;

        /// the current point of focus (usually the canvas center)
        QPointF posFocus;

        /// the current mouse handler
        IMouse * mouse;

        /// tool tip timer for vector map tool tips
        QTimer * timerToolTip;

        /// the position of the tool tip
        QPoint posToolTip;

        QMovie * loadIndicator;
        QLabel * mapLoadIndicator;
};

#endif //CCANVAS_H

