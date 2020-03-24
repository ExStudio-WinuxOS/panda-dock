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

#include "mainwindow.h"
#include "utils/utils.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QScreen>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>

#include <KF5/KWindowSystem/KWindowEffects>

#define MAINWINDOW_MAX_SIZE       100
#define MAINWINDOW_MIN_SIZE       (45)
#define MAINWINDOW_PADDING (0)
#define DRAG_AREA_SIZE (5)

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_mainPanel(new MainPanel),
      m_itemManager(DockItemManager::instance()),
      m_settings(DockSettings::instance()),
//      m_xcbMisc(XcbMisc::instance()),
      m_fakeWidget(new QWidget(nullptr))
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_mainPanel);
    setLayout(layout);

    m_fakeWidget->setFocusPolicy(Qt::NoFocus);
    m_fakeWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_fakeWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_fakeWidget->setVisible(false);

    for (auto item : m_itemManager->itemList())
        m_mainPanel->insertItem(-1, item);

    initWindowSize();

    // blur
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);

    KWindowSystem::setOnDesktop(effectiveWinId(), NET::OnAllDesktops);
    KWindowSystem::setType(winId(), NET::Dock);
    // KWindowEffects::slideWindow(winId(), KWindowEffects::BottomEdge);

    connect(m_settings, &DockSettings::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_settings, &DockSettings::iconSizeChanged, this, &MainWindow::initWindowSize);
    connect(m_mainPanel, &MainPanel::requestResized, this, &MainWindow::initWindowSize);

    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &MainWindow::initWindowSize, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{

}

void MainWindow::initWindowSize()
{
    QRect windowRect = m_settings->windowRect();

    // set size.
    QWidget::move(windowRect.left(), windowRect.top());
    QWidget::setFixedSize(windowRect.size());

    QTimer::singleShot(100, this, [=] {
        qreal radius;
        if (m_settings->position() == DockSettings::Bottom) {
            radius = this->rect().height() * 0.35;
        } else {
            radius = this->rect().width() * 0.35;
        }
        QPainterPath path;
        path.addRoundedRect(this->rect(), radius, radius);
        KWindowEffects::enableBlurBehind(winId(), true, path.toFillPolygon().toPolygon());

//        int backgroundOpacity = 150;
//        const qreal factor = (qreal)backgroundOpacity / (qreal)100;
//        qreal contrast = currentMidValue(0.45, factor, 1);
//        qreal intesity = currentMidValue(0.45, factor, 1);
//        qreal saturation = currentMidValue(1.7, factor, 1);
//        KWindowEffects::enableBackgroundContrast(winId(), true, 0.45, 0.45, 1.7, path.toFillPolygon().toPolygon());
    });

    m_size = windowRect.size();

    setStrutPartial();
}

void MainWindow::setStrutPartial()
{
    // 不清真的作法，kwin设置blur后设置程序支撑导致模糊无效
    QRect rect(geometry());
    rect.setHeight(1);
    rect.setWidth(1);
    m_fakeWidget->setGeometry(rect);
    m_fakeWidget->setVisible(true);
    KWindowSystem::setState(m_fakeWidget->winId(), NET::SkipTaskbar);
    KWindowSystem::setState(m_fakeWidget->winId(), NET::SkipSwitcher);

    const auto ratio = devicePixelRatioF();
    const int margin = 14;

    NETExtendedStrut strut;

    switch (m_settings->position()) {
    case DockSettings::Left:
        strut.left_width = width() + margin * 2;
        strut.left_start = y();
        strut.left_end = y() + height() - 1;
        break;
    case DockSettings::Right:
        strut.right_width = width() + margin * 2;
        strut.right_start = y();
        strut.right_end = y() + height() - 1;
        break;
    case DockSettings::Bottom:
        strut.bottom_width = height() + margin * 2;
        strut.bottom_start = x();
        strut.bottom_end = x() + width();
        break;
    }

    KWindowSystem::setExtendedStrut(m_fakeWidget->winId(),
                                     strut.left_width,
                                     strut.left_start,
                                     strut.left_end,
                                     strut.right_width,
                                     strut.right_start,
                                     strut.right_end,
                                     strut.top_width,
                                     strut.top_start,
                                     strut.top_end,
                                     strut.bottom_width,
                                     strut.bottom_start,
                                     strut.bottom_end);
}

void MainWindow::onPositionChanged()
{
    initWindowSize();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void MainWindow::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    //XcbMisc::instance()->enableBlurBehind(winId(), true);
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
//    QColor color("#000000");
//    color.setAlpha(60);
    QColor color("#FFFFFF");
    color.setAlpha(80); // default 130
    painter.setPen(QColor(0, 0, 0, 30));
    painter.setBrush(color);

    int radius;
    if (m_settings->position() == DockSettings::Bottom) {
        radius = rect().height() * 0.35;
    } else {
        radius = rect().width() * 0.35;
    }
    painter.drawRoundedRect(rect(), radius, radius);
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    e->ignore();

    if (e->button() == Qt::RightButton) {
        m_settings->showSettingsMenu();
    }
}