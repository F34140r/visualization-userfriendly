/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <QPainter>
#include <QStyleOptionViewItem>

#include "rviz/properties/color_item.h"
#include "rviz/properties/color_editor.h"
#include "rviz/properties/property.h"

namespace rviz
{

ColorItem::ColorItem( ColorProperty* property )
  : PropertyWidgetItem( property, property->getName(), property->hasSetter() )
{}

bool ColorItem::paint( QPainter* painter, const QStyleOptionViewItem& option )
{
  QColor color = userData().value<QColor>();
  QString text = QString("%1, %2, %3").arg( color.red() ).arg( color.green() ).arg( color.blue() );
  QRect rect = option.rect;
  ColorEditor::paintColorBox( painter, rect, color );
  rect.adjust( rect.height() + 1, 1, 0, 0 );
  painter->drawText( rect, text );

  return true; // return true, since this function has done the painting.
}

QWidget* ColorItem::createEditor( QWidget* parent, const QStyleOptionViewItem & option )
{
  ColorEditor* editor = new ColorEditor( parent );
  editor->setFrame( false );
  return editor;
}

bool ColorItem::setEditorData( QWidget* editor )
{
  printf("ColorItem::setEditorData()\n");
  if( ColorEditor* color_editor = qobject_cast<ColorEditor*>( editor ))
  {
    QColor color = userData().value<QColor>();
    printf("ColorItem::setEditorData() color = %d, %d, %d\n", color.red(), color.green(), color.blue());
    color_editor->setColor( userData().value<QColor>() );
    return true;
  }
  return false;
}

bool ColorItem::setModelData( QWidget* editor )
{
  printf("ColorItem::setModelData()\n");
  if( ColorEditor* color_editor = qobject_cast<ColorEditor*>( editor ))
  {
    if( color_editor->isModified() )
    {
      printf("ColorItem::setModelData() calling setData( %d %d %d )\n",
             color_editor->getColor().red(),
             color_editor->getColor().green(),
             color_editor->getColor().blue() );
      setData( 1, Qt::UserRole, QVariant::fromValue( color_editor->getColor() ));
    }
    return true;
  }
  return false;
}

} // end namespace rviz
