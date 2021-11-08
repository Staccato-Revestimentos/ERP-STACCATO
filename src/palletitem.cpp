#include "palletitem.h"

#include "application.h"
#include "sqlquery.h"

#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMimeData>
#include <QPainter>
#include <QScrollArea>
#include <QSqlError>
#include <QToolTip>

PalletItem::PalletItem(const QString &idBloco, const QString &label, const QPointF &posicao, const QRectF &size, const int sceneSize, QGraphicsItem *parent)
    : QGraphicsObject(parent), sceneSize(sceneSize + 79), size(size), idBloco(idBloco), label(label) {
  setAcceptHoverEvents(true);
  setAcceptDrops(true);

  setPos(posicao);
}

QRectF PalletItem::boundingRect() const { return size; }

void PalletItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)

  if (flagHighlight) {
    painter->setPen(QColor(Qt::blue));
    painter->setBrush(QBrush(QColor(Qt::black)));
    painter->drawRect(size);
  } else {
    painter->setPen(QColor(Qt::black));
    painter->drawRect(size);
  }

  if (selected) {
    painter->setBrush(QBrush(QColor(Qt::black), Qt::SolidPattern));
    painter->drawRect(size);
  }

  if (not label.isEmpty()) {
    QFont font = painter->font();
    font.setPixelSize((size.size().width() < 20 or size.size().height() < 20) ? 7 : 14);
    painter->setFont(font);

    QFontMetrics fm(painter->font());
    const auto xOffset = fm.boundingRect(label).width() / 2;
    const auto yOffset = fm.boundingRect(label).height() / 2;
    const auto xCenter = size.center().x();
    const auto yCenter = size.center().y();

    painter->setPen(QColor(Qt::red));
    painter->drawText(xCenter - xOffset, yCenter + yOffset, label);
  }
}

void PalletItem::addEstoque(const QString &estoqueText) {
  auto *estoque = new EstoqueItem(estoqueText, this);
  estoque->setVisible(false);
  const int offset = counter++ * 15;
  estoque->setPos(mapFromScene(sceneSize, offset));

  if (estoques.isEmpty()) { estoques += "\n"; }

  estoques += estoqueText;
}

void PalletItem::setLabel(const QString &value) { label = value; }

QString PalletItem::getLabel() const { return label; }

QString PalletItem::getEstoques() const { return estoques; }

bool PalletItem::getFlagHighlight() const { return flagHighlight; }

void PalletItem::setFlagHighlight(const bool value) { flagHighlight = value; }

void PalletItem::reorderChildren() {
  int pos = 15;

  const auto children = childItems();

  for (auto *const estoque : children) {
    estoque->setPos(mapFromScene(sceneSize, pos));
    pos += 15;
  }
}

void PalletItem::unselect() {
  if (not selected) { return; }

  selected = false;

  prepareGeometryChange();

  const auto children = childItems();

  for (auto *const estoque : children) { estoque->setVisible(false); }
}

const QRectF &PalletItem::getSize() const { return size; }

void PalletItem::setSize(const QRectF &newSize) {
  prepareGeometryChange();
  size = newSize;
}

void PalletItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) { QGraphicsItem::hoverEnterEvent(event); }

void PalletItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event) { QGraphicsItem::hoverMoveEvent(event); }

void PalletItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) { QGraphicsItem::hoverLeaveEvent(event); }

void PalletItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  //  qDebug() << "press";

  if (not flags().testFlag(QGraphicsItem::ItemIsSelectable)) { return; }

  if (not selected) { emit unselectOthers(); }

  selected = not selected;

  prepareGeometryChange();

  const auto children = childItems();

  for (auto *const estoque : children) { estoque->setVisible(not estoque->isVisible()); }

  QGraphicsItem::mousePressEvent(event);
}

void PalletItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  //  qDebug() << "move";
  QGraphicsItem::mouseMoveEvent(event);
}

void PalletItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  //  qDebug() << "release";

  reorderChildren();

  QGraphicsItem::mouseReleaseEvent(event);
}

QString PalletItem::getIdBloco() const { return idBloco; }

void PalletItem::setIdBloco(const QString &newIdBloco) { idBloco = newIdBloco; }

QString PalletItem::getPosicao() const { return QString::number(scenePos().x()) + "," + QString::number(scenePos().y()); }

QString PalletItem::getTamanho() const { return QString::number(size.width()) + "," + QString::number(size.height()); }

void PalletItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) { QGraphicsItem::mouseDoubleClickEvent(event); }

void PalletItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event) {
  if (childItems().contains(qobject_cast<EstoqueItem *>(event->mimeData()->parent()))) {
    event->ignore();
    return;
  }

  if (event->mimeData()->hasFormat("text/plain")) { event->accept(); }
}

void PalletItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event) { Q_UNUSED(event); }

void PalletItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event) { Q_UNUSED(event); }

void PalletItem::dropEvent(QGraphicsSceneDragDropEvent *event) {
  Q_UNUSED(event);

  const QStringList text = event->mimeData()->text().split(" - ", Qt::SkipEmptyParts);

  if (text.isEmpty()) { return; }

  const QString &id = text.at(0);
  const QString table = (text.at(1) == "ESTOQUE") ? "estoque" : "estoque_has_consumo";
  const QString idName = (table == "estoque") ? "idEstoque" : "idConsumo";

  SqlQuery query;

  if (not query.exec("UPDATE " + table + " SET bloco = '" + label + "' WHERE " + idName + " = " + id)) {
    event->ignore();
    throw RuntimeError("Erro movendo estoque: " + query.lastError().text());
  }

  auto *item = qobject_cast<EstoqueItem *>(event->mimeData()->parent());
  item->setParentItem(this);
  item->setVisible(false);
  reorderChildren();

  event->acceptProposedAction();
}

void PalletItem::select() {
  selected = not selected;

  prepareGeometryChange();

  const auto children = childItems();

  for (auto *const estoque : children) { estoque->setVisible(not estoque->isVisible()); }
}

// TODO: deletar pallet usando seleção e depois apertando Delete, transferir o conteudo do pallet para a entrada ou um espaço temporario
