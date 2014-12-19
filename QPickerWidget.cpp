//
//   QPickerWidget.cpp
//
//   Copyright (c) 2014 Dale Low
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "QPickerWidget.h"

#define PICKER_LABEL_INSET          2
#define MIN_MOVE_AMOUNT             2

///////////////////////////////////////////////////////////////////////////////

QPickerWidget::QPickerWidget(const QRect &frame, int id, int pickerItemHeight, const QList<QString> &values,
        const QString &backgroundStyleClass, const QPixmap &backgroundPixmap, const QPixmap &overlayPixmap, const QString &valueStyleClass,
        const QString &targetLineStyleClass, const QString &targetValueStyleClass, QWidget *parent) :
                QLabel(parent), m_id(id), m_pickerItemHeight(pickerItemHeight), m_valueStyleClass(valueStyleClass), m_targetValueStyleClass(targetValueStyleClass)
{
    setGeometry(frame);

    if (backgroundPixmap.isNull()) {
        m_hasBackgroundStyleClass = true;
        setProperty("class", backgroundStyleClass);
    } else {
        m_hasBackgroundStyleClass = false;
        setPixmap(backgroundPixmap);
        setScaledContents(true);
    }

    m_scrollEntryCount = values.size();
    if (!m_scrollEntryCount) {
        return;
    }

    m_scrollWidget = new QWidget(this);
    m_scrollWidget->setGeometry(0, 0, frame.width(), m_scrollEntryCount * m_pickerItemHeight);

    int labelWidth = frame.width() - PICKER_LABEL_INSET*2;
    m_pickerLabels = (QLabel **)malloc(m_scrollEntryCount * sizeof(QLabel *));
    for (int i=0; i<m_scrollEntryCount; i++) {
        QLabel *label = m_pickerLabels[i] = new QLabel(m_scrollWidget);
        label->setProperty("class", valueStyleClass);
        label->setAlignment(Qt::AlignCenter);
        label->setGeometry(PICKER_LABEL_INSET, i*m_pickerItemHeight + PICKER_LABEL_INSET, labelWidth, m_pickerItemHeight - PICKER_LABEL_INSET*2);
        label->setText(values[i]);
    }

    // target lines
    int topY = frame.height()/2 - m_pickerItemHeight/2;
    int bottomY = frame.height()/2 + m_pickerItemHeight/2;

    QWidget *targetLine = new QWidget(this);
    targetLine->setGeometry(0, topY, frame.width(), 1);
    targetLine->setProperty("class", targetLineStyleClass);

    targetLine = new QWidget(this);
    targetLine->setGeometry(0, bottomY, frame.width(), 1);
    targetLine->setProperty("class", targetLineStyleClass);

    if (!overlayPixmap.isNull()) {
        QLabel *overlay = new QLabel(this);
        overlay->setStyleSheet("background-color: transparent");
        overlay->setGeometry(this->rect());
        overlay->setScaledContents(true);
        overlay->setPixmap(overlayPixmap);
        overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    // min/max values for scrolling
    m_ymin = bottomY - m_scrollEntryCount*m_pickerItemHeight;
    m_ymax = topY;

    // start at first entry
    m_index = 0;
    m_scrollWidget->move(0, m_ymax);
    m_pickerLabels[m_index]->setProperty("class", m_targetValueStyleClass);
}

QPickerWidget::~QPickerWidget()
{
    free(m_pickerLabels);
}

void QPickerWidget::setIndex(int index)
{
    if (index == m_index) {
        return;
    }

    updateStyleClassForPickerLabel(m_index, m_valueStyleClass);

    m_index = MIN(MAX(index, 0), m_scrollEntryCount-1);
    m_scrollWidget->move(0, m_ymax - m_index*m_pickerItemHeight);
    updateStyleClassForPickerLabel(m_index, m_targetValueStyleClass);
}

int QPickerWidget::index() const
{
    return m_index;
}

QString QPickerWidget::currentValue() const
{
    return m_pickerLabels[m_index]->text();
}

#pragma mark - internal methods

void inline QPickerWidget::updateStyleClassForPickerLabel(int index, const QString &styleClass)
{
    m_pickerLabels[index]->setProperty("class", styleClass);

    // when modifying the class, we need to force the widget to update (https://qt-project.org/doc/qt-4.7/stylesheet-examples.html)
    m_pickerLabels[index]->style()->unpolish(m_pickerLabels[index]);
    m_pickerLabels[index]->style()->polish(m_pickerLabels[index]);
}

// credit: http://stackoverflow.com/questions/18344135/why-do-stylesheets-not-work-when-subclassing-qwidget-and-using-q-object
void QPickerWidget::paintEvent(QPaintEvent *event)
{
    if (!m_hasBackgroundStyleClass) {
        return QLabel::paintEvent(event);
    }

    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);
}

void QPickerWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastY = event->y();
    m_targetY = m_scrollWidget->pos().y();

    // unhighlight the selected row
    updateStyleClassForPickerLabel(m_index, m_valueStyleClass);

    m_handledMouseMoveEvent = false;
}

void QPickerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (abs(event->y() - m_lastY) > MIN_MOVE_AMOUNT) {
        int deltaY = event->y() - m_lastY;

        m_targetY += deltaY;
        m_scrollWidget->move(0, m_targetY);

        m_lastY = event->y();
        m_handledMouseMoveEvent = true;
    }
}

void QPickerWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_handledMouseMoveEvent) {
        QPoint pos = m_scrollWidget->pos();
        int y = pos.y();

        // snap to bottom, top or closest position (unfortunately, most embedded platforms don't support QPropertyAnimation)
        if (y < m_ymin) {
            pos.setY(m_ymin);
        } else if (y > m_ymax) {
            pos.setY(m_ymax);
        } else {
            pos.setY(((y - m_ymin + m_pickerItemHeight/2)/m_pickerItemHeight)*m_pickerItemHeight + m_ymin);
        }

        m_scrollWidget->move(pos);

        int newIndex = (m_ymax - m_scrollWidget->pos().y())/m_pickerItemHeight;

        // always need to do this since we unhighlighted it when we got the press down event
        updateStyleClassForPickerLabel(newIndex, m_targetValueStyleClass);

        if (newIndex != m_index) {
            m_index = newIndex;
            emit moved(m_id, m_index);
        }
    }
}
