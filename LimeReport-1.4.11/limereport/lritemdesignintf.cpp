/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "lritemdesignintf.h"
#include "lrbasedesignintf.h"
#include "lrpagedesignintf.h"
#include "lrdatasourcemanager.h"
#include "lrscriptenginemanager.h"
#include "lrglobal.h"

#include <QDebug>
#include <QRegExp>
#include <QVariant>

namespace LimeReport{

ItemDesignIntf::ItemDesignIntf(const QString &xmlTypeName, QObject* owner, QGraphicsItem* parent) :
    BaseDesignIntf(xmlTypeName,owner,parent), m_itemLocation(Band), m_stretchToMaxHeight(false){
    initFlags();
}

void ItemDesignIntf::setItemLocation(LocationType location)
{

    if (m_itemLocation!=location){
        LocationType oldValue = m_itemLocation;
        m_itemLocation=location;
        if(!isLoading()){
            if (location==Band){
                QGraphicsItem *parentBand=bandByPos();
                if (parentBand){
                    QPointF parentPos = parentBand->mapFromItem(parentItem(),x(),y());
                    setParentItem(parentBand);
                    setParent(dynamic_cast<BandDesignIntf*>(parentBand));
                    setPos(parentPos);
                } else {
                    m_itemLocation=Page;
                }
            } else {
                if (scene()){
                    PageItemDesignIntf* page = dynamic_cast<PageDesignIntf*>(scene())->pageItem();
                    QPointF parentPos = page->mapFromItem(parentItem(),x(),y());
                    setParentItem(page);
                    setParent(page);
                    setPos(parentPos);
                }
            }
            notify("locationType",oldValue,location);
        }
    }
}

void ItemDesignIntf::setStretchToMaxHeight(bool value)
{
    if (m_stretchToMaxHeight!=value){
        bool oldValue = m_stretchToMaxHeight;
        m_stretchToMaxHeight=value;
        notify("stretchToMaxHeight",oldValue,value);
    }
}

BaseDesignIntf *ItemDesignIntf::cloneEmpty(int height, QObject *owner, QGraphicsItem *parent)
{
    BaseDesignIntf* spacer = new Spacer(owner,parent);
    spacer->initFromItem(this);
    spacer->setHeight(height);
    return spacer;
}

QGraphicsItem * ItemDesignIntf::bandByPos()
{
    foreach(QGraphicsItem *item,collidingItems()){
        if (dynamic_cast<BandDesignIntf*>(item)){
            return item;
        }
    }
    return nullptr;
}

void ItemDesignIntf::initFlags()
{
    BaseDesignIntf::initFlags();
    if ((itemMode()&DesignMode) || (itemMode()&EditMode)){
        setFlag(QGraphicsItem::ItemIsMovable);
    } else {
        setFlag(QGraphicsItem::ItemIsMovable,false);
    }
}

Spacer::Spacer(QObject *owner, QGraphicsItem *parent)
    :ItemDesignIntf("Spacer",owner,parent){}

}// namespace LimeReport
