/*
 * Copyright (C) 2020 PandaOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPITEM_H
#define APPITEM_H

#include "dockitem.h"
#include "blurwindow.h"
#include "utils/dockitemmanager.h"
#include "utils/appwindowmanager.h"

#include <QVariantAnimation>
#include <QDrag>

class AppItem : public DockItem
{
    Q_OBJECT

public:
    explicit AppItem(DockEntry *entry, QWidget *parent = nullptr);
    ~AppItem();

    inline ItemType itemType() const override { return DockItem::App; }
    QString popupText() override;
    DockEntry *entry();

    void open();
    void close();
    void update();

    void setBlockMouseRelease(bool enable);
    void delayUpdateIconGeomerty();

signals:
    void dragStarted();

private:
    void updateIconGeometry();
    void initDockAction();
    void dockActionTriggered();
    void updateIcon();
    void initStates();
    void startDrag();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    QPixmap m_iconPixmap;
    DockEntry *m_entry;

    QMenu m_contextMenu;
    QAction *m_appNameAction;
    QAction *m_openAction;
    QAction *m_closeAction;
    QAction *m_dockAction;
    QTimer *m_updateIconGeometryTimer;
    QTimer *m_dragActiveTimer;

    QDrag *m_drag;
    QPoint m_mousePressPos;
    bool m_dragging = false;

    bool m_blockMouseRelease = false;
};

#endif // APPITEM_H
