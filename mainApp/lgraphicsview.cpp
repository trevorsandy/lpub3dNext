/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtOpenGL>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include "lgraphicsview.h"
#include "lpub.h"

LGraphicsView::LGraphicsView(LGraphicsScene *scene)
{
  setAcceptDrops(true);
  setScene(scene);
  pageBackgroundItem = NULL;
  fitMode  = FitVisible;
  pageRect = QRectF(0,0,0,0);

  /*
   * View Ruler Class
   *
   */
  setViewportMargins(RULER_BREADTH,RULER_BREADTH,0,0);
  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->setSpacing(0);
  gridLayout->setMargin(0);

  QWidget* fake = new QWidget();
  fake->setBackgroundRole(QPalette::Window);
  fake->setFixedSize(RULER_BREADTH,RULER_BREADTH);

  LRuler * mHorzRuler = new LRuler(LRuler::Horizontal,fake);
  LRuler * mVertRuler = new LRuler(LRuler::Vertical,fake);

  gridLayout->addWidget(fake,0,0);
  gridLayout->addWidget(mHorzRuler,0,1);
  gridLayout->addWidget(mVertRuler,1,0);
  gridLayout->addWidget(this->viewport(),1,1);

  this->setLayout(gridLayout);
   /*
   */
}

void LGraphicsView::fitVisible(const QRectF rect)
{
  scale(1.0,1.0);
  pageRect = rect;

  QRectF unity = matrix().mapRect(QRectF(0,0,1,1));
  scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = matrix().mapRect(pageRect);
  qreal xratio = viewRect.width() / sceneRect.width();
  qreal yratio = viewRect.height() / sceneRect.height();

  xratio = yratio = qMin(xratio,yratio);
  scale(xratio,yratio);
  centerOn(pageRect.center());
  fitMode = FitVisible;
}

void LGraphicsView::fitWidth(const QRectF rect)
{
  scale(1.0,1.0);
  pageRect = rect;

  QRectF unity = matrix().mapRect(QRectF(0,0,1,1));
  scale(1 / unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = matrix().mapRect(pageRect);
  qreal xratio = viewRect.width() / sceneRect.width();

  scale(xratio,xratio);
  centerOn(pageRect.center());
  fitMode = FitWidth;
}

void LGraphicsView::actualSize(){
  resetMatrix();
  fitMode = FitNone;
}

void LGraphicsView::zoomIn(){
  scale(1.1,1.1);
  fitMode = FitNone;
}

void LGraphicsView::zoomOut(){
  scale(1.0/1.1,1.0/1.1);
  fitMode = FitNone;
}

void LGraphicsView::dragMoveEvent(QDragMoveEvent *event){
  if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
}

void LGraphicsView::dragEnterEvent(QDragEnterEvent *event){
  if (event->mimeData()->hasUrls()) {
      event->acceptProposedAction();
    }
}

void LGraphicsView::dragLeaveEvent(QDragLeaveEvent *event){
  event->accept();
}

void LGraphicsView::dropEvent(QDropEvent *event){
  const QMimeData* mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
      QList<QUrl> urlList = mimeData->urls();
      QString fileName = urlList.at(0).toLocalFile();   // load the first file only
      if (urlList.size() > 1) {
          QMessageBox::warning(NULL,
                               QMessageBox::tr(VER_PRODUCTNAME_STR),
                               QMessageBox::tr("%1 files selected.\nOnly file %2 will be opened.")
                               .arg(urlList.size())
                               .arg(fileName));
        }
      gui->openDropFile(fileName);
      event->acceptProposedAction();
    }
}

void LGraphicsView::resizeEvent(QResizeEvent *event)
{
  Q_UNUSED(event);
  if (pageBackgroundItem) {
      if (fitMode == FitVisible) {
          fitVisible(pageRect);
        } else if (fitMode == FitWidth) {
          fitWidth(pageRect);
        }
    }
}


/*
 * View Ruler class
 *
 */
void LRuler::setOrigin(const qreal origin)
{
  if (mOrigin != origin)
  {
    mOrigin = origin;
    update();
  }
}

void LRuler::setRulerUnit(const qreal rulerUnit)
{
  if (mRulerUnit != rulerUnit)
  {
    mRulerUnit = rulerUnit;
    update();
  }
}

void LRuler::setRulerZoom(const qreal rulerZoom)
{
  if (mRulerZoom != rulerZoom)
  {
    mRulerZoom = rulerZoom;
    update();
  }
}


void LRuler::setCursorPos(const QPoint cursorPos)
{
  mCursorPos = this->mapFromGlobal(cursorPos);
  mCursorPos += QPoint(RULER_BREADTH,RULER_BREADTH);
  update();
}

void LRuler::setMouseTrack(const bool track)
{
  if (mMouseTracking != track)
  {
    mMouseTracking = track;
    update();
  }
}

void LRuler::mouseMoveEvent(QMouseEvent* event)
{
  mCursorPos = event->pos();
  update();
  QWidget::mouseMoveEvent(event);
}

void LRuler::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    QPen pen(Qt::black,0); // zero width pen is cosmetic pen
    //pen.setCosmetic(true);
    painter.setPen(pen);
  // We want to work with floating point, so we are considering
  // the rect as QRectF
  QRectF rulerRect = this->rect();

  // at first fill the rect
  //painter.fillRect(rulerRect,QColor(220,200,180));
  //painter.fillRect(rulerRect,QColor(236,233,216));
  painter.fillRect(rulerRect,QColor(Qt::lightGray));

  // drawing a scale of 25
  drawAScaleMeter(&painter,rulerRect,25,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/2);
  // drawing a scale of 50
  drawAScaleMeter(&painter,rulerRect,50,(Horizontal == mRulerType ? rulerRect.height()
        : rulerRect.width())/4);
  // drawing a scale of 100
  mDrawText = true;
  drawAScaleMeter(&painter,rulerRect,100,0);
  mDrawText = false;

  // drawing the current mouse position indicator
    painter.setOpacity(0.4);
    drawMousePosTick(&painter);
    painter.setOpacity(1.0);

  // drawing no man's land between the ruler & view
  QPointF starPt = Horizontal == mRulerType ? rulerRect.bottomLeft()
      : rulerRect.topRight();
  QPointF endPt = Horizontal == mRulerType ? rulerRect.bottomRight()
      : rulerRect.bottomRight();
  painter.setPen(QPen(Qt::black,2));
  painter.drawLine(starPt,endPt);
}

void LRuler::drawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin)
{
  // Flagging whether we are horizontal or vertical only to reduce
  // to cheching many times
  bool isHorzRuler = Horizontal == mRulerType;

  scaleMeter  = scaleMeter * mRulerUnit * mRulerZoom;

  // Ruler rectangle starting mark
  qreal rulerStartMark = isHorzRuler ? rulerRect.left() : rulerRect.top();
  // Ruler rectangle ending mark
  qreal rulerEndMark = isHorzRuler ? rulerRect.right() : rulerRect.bottom();

  // Condition A # If origin point is between the start & end mark,
  //we have to draw both from origin to left mark & origin to right mark.
  // Condition B # If origin point is left of the start mark, we have to draw
  // from origin to end mark.
  // Condition C # If origin point is right of the end mark, we have to draw
  // from origin to start mark.
  if (mOrigin >= rulerStartMark && mOrigin <= rulerEndMark)
  {
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerEndMark, 0, scaleMeter, startPositoin);
    drawFromOriginTo(painter, rulerRect, mOrigin, rulerStartMark, 0, -scaleMeter, startPositoin);
  }
  else if (mOrigin < rulerStartMark)
  {
        int tickNo = int((rulerStartMark - mOrigin) / scaleMeter);
        drawFromOriginTo(painter, rulerRect, mOrigin + scaleMeter * tickNo,
            rulerEndMark, tickNo, scaleMeter, startPositoin);
  }
  else if (mOrigin > rulerEndMark)
  {
        int tickNo = int((mOrigin - rulerEndMark) / scaleMeter);
    drawFromOriginTo(painter, rulerRect, mOrigin - scaleMeter * tickNo,
            rulerStartMark, tickNo, -scaleMeter, startPositoin);
  }
}

void LRuler::drawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition)
{
  bool isHorzRuler = Horizontal == mRulerType;
  int iterate = 0;

  for (qreal current = startMark;
      (step < 0 ? current >= endMark : current <= endMark); current += step)
  {
    qreal x1 = isHorzRuler ? current : rulerRect.left() + startPosition;
    qreal y1 = isHorzRuler ? rulerRect.top() + startPosition : current;
    qreal x2 = isHorzRuler ? current : rulerRect.right();
    qreal y2 = isHorzRuler ? rulerRect.bottom() : current;
    painter->drawLine(QLineF(x1,y1,x2,y2));
    if (mDrawText)
    {
      QPainterPath txtPath;
            txtPath.addText(x1 + 1,y1 + (isHorzRuler ? 7 : -2),this->font(),QString::number(qAbs(int(step) * startTickNo++)));
      painter->drawPath(txtPath);
      iterate++;
    }
  }
}

void LRuler::drawMousePosTick(QPainter* painter)
{
  if (mMouseTracking)
  {
    QPoint starPt = mCursorPos;
    QPoint endPt;
    if (Horizontal == mRulerType)
    {
      starPt.setY(this->rect().top());
      endPt.setX(starPt.x());
      endPt.setY(this->rect().bottom());
    }
    else
    {
      starPt.setX(this->rect().left());
      endPt.setX(this->rect().right());
      endPt.setY(starPt.y());
    }
    painter->drawLine(starPt,endPt);
  }
}
/*
*/
