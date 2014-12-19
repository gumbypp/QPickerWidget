//
//   QPickerWidget.h
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

#include <QtGui>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

///////////////////////////////////////////////////////////////////////////////

class QPickerWidget : public QLabel
{
    Q_OBJECT

public:
    QPickerWidget(const QRect &frame, int id, int pickerItemHeight, const QList<QString> &values,
            const QString &backgroundStyleClass, const QPixmap &backgroundPixmap, const QPixmap &overlayPixmap, const QString &valueStyleClass,
            const QString &targetLineStyleClass, const QString &targetValueStyleClass, QWidget *parent = 0);
    virtual ~QPickerWidget();
    void setIndex(int index);
    int index() const;
    QString currentValue() const;

signals:
    void moved(int id, int index);

protected:
    void inline updateStyleClassForPickerLabel(int index, const QString &styleClass);
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

protected:
    int m_id;
    int m_pickerItemHeight;
    bool m_hasBackgroundStyleClass;
    QString m_valueStyleClass;
    QString m_targetValueStyleClass;
    QWidget *m_scrollWidget;
    QLabel **m_pickerLabels; // weak references to labels on the scroll widget
    int m_scrollEntryCount;
    int m_index;
    int m_lastY;
    int m_targetY;
    bool m_handledMouseMoveEvent;
    int m_ymin;
    int m_ymax;
};
