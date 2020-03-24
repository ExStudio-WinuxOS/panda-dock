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

#include "trashitem.h"
#include "utils/utils.h"
#include <QMouseEvent>
#include <QProcess>
#include <QPainter>
#include <QDialog>
#include <QDir>

const QString TrashDir = QDir::homePath() + "/.local/share/Trash";
const QDir::Filters ItemsShouldCount = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

TrashItem::TrashItem(QWidget *parent)
    : DockItem(parent),
      m_filesWatcher(new QFileSystemWatcher(this)),
      m_rightMenu(new QMenu(this))
{
    QAction *openAction = new QAction(tr("Open"));
    QAction *emptyAction = new QAction(tr("Empty"));

    m_rightMenu->addAction(openAction);
    m_rightMenu->addAction(emptyAction);

    connect(m_filesWatcher, &QFileSystemWatcher::directoryChanged, this, &TrashItem::onDirectoryChanged, Qt::QueuedConnection);
    connect(openAction, &QAction::triggered, this, &TrashItem::openTrashFold);
    connect(emptyAction, &QAction::triggered, this, &TrashItem::emptyTrash);

    setAccessibleName("Trash");
    onDirectoryChanged();
}

void TrashItem::refreshIcon()
{
    const int iconSize = qMin(width(), height());

    if (m_count) {
        m_iconPixmap = Utils::renderSVG(":/resources/user-trash-full.svg", QSize(iconSize * 0.8, iconSize * 0.8));
    } else {
        m_iconPixmap = Utils::renderSVG(":/resources/user-trash.svg", QSize(iconSize * 0.8, iconSize * 0.8));
    }

    QWidget::update();
}

void TrashItem::onDirectoryChanged()
{
    m_filesWatcher->addPath(TrashDir);

    if (QDir(TrashDir + "/files").exists()) {
        m_filesWatcher->addPath(TrashDir + "/files");
        m_count = QDir(TrashDir + "/files").entryList(ItemsShouldCount).count();
    } else {
        m_count = 0;
    }

    refreshIcon();
}

void TrashItem::openTrashFold()
{
    QProcess::startDetached("gio", QStringList() << "open" << "trash:///");
}

void TrashItem::emptyTrash()
{
//    QDialog *dialog = new QDialog;
//    dialog->setWindowTitle(tr("Empty Trash"));
//    dialog->show();
}

void TrashItem::paintEvent(QPaintEvent *e)
{
    DockItem::paintEvent(e);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const auto ratio = devicePixelRatioF();
    const int iconX = rect().center().x() - m_iconPixmap.rect().center().x() / ratio;
    const int iconY = rect().center().y() - m_iconPixmap.rect().center().y() / ratio;

    painter.drawPixmap(iconX, iconY, m_iconPixmap);
}

void TrashItem::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    refreshIcon();
}

void TrashItem::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {
        m_rightMenu->move(QCursor::pos());
        m_rightMenu->exec();
    }
}

void TrashItem::mouseReleaseEvent(QMouseEvent *e)
{
    openTrashFold();
    QWidget::mouseReleaseEvent(e);
}