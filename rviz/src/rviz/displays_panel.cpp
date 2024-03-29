/*
 * Copyright (c) 2008, Willow Garage, Inc.
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

#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QApplication>

#include <boost/bind.hpp>

#include "rviz/display_factory.h"
#include "rviz/display.h"
#include "rviz/new_object_dialog.h"
#include "rviz/properties/property.h"
#include "rviz/properties/property_tree_widget.h"
#include "rviz/properties/property_tree_with_help.h"
#include "rviz/visualization_manager.h"

#include "rviz/displays_panel.h"

static const std::string PROPERTY_GRID_CONFIG("Property Grid State");
static const std::string PROPERTY_GRID_SPLITTER("Property Grid Splitter");

namespace rviz
{

DisplaysPanel::DisplaysPanel( QWidget* parent )
  : QWidget( parent )
  , manager_( NULL )
{
  tree_with_help_ = new PropertyTreeWithHelp;
  property_grid_ = tree_with_help_->getTree();

  QPushButton* add_button = new QPushButton( "Add" );
  add_button->setShortcut( QKeySequence( QString( "Ctrl+N" )));
  add_button->setToolTip( "Add a new display, Ctrl+N" );
  remove_button_ = new QPushButton( "Remove" );
  remove_button_->setShortcut( QKeySequence( QString( "Ctrl+X" )));
  remove_button_->setToolTip( "Remove displays, Ctrl+X" );
  remove_button_->setEnabled( false );
  rename_button_ = new QPushButton( "Rename" );
  rename_button_->setShortcut( QKeySequence( QString( "Ctrl+R" )));
  rename_button_->setToolTip( "Rename a display, Ctrl+R" );
  rename_button_->setEnabled( false );

  QHBoxLayout* button_layout = new QHBoxLayout;
  button_layout->addWidget( add_button );
  button_layout->addWidget( remove_button_ );
  button_layout->addWidget( rename_button_ );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget( tree_with_help_ );
  layout->addLayout( button_layout );

  setLayout( layout );

  connect( add_button, SIGNAL( clicked( bool )), this, SLOT( onNewDisplay() ));
  connect( remove_button_, SIGNAL( clicked( bool )), this, SLOT( onDeleteDisplay() ));
  connect( rename_button_, SIGNAL( clicked( bool )), this, SLOT( onRenameDisplay() ));
  connect( property_grid_, SIGNAL( selectionHasChanged() ), this, SLOT( onSelectionChanged() ));
}

DisplaysPanel::~DisplaysPanel()
{
}

void DisplaysPanel::initialize( VisualizationManager* manager )
{
  manager_ = manager;
  property_grid_->setModel( manager_->getDisplayTreeModel() );
}

void DisplaysPanel::onNewDisplay()
{
  QString lookup_name;
  QString display_name;

  QStringList empty;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  NewObjectDialog* dialog = new NewObjectDialog( manager_->getDisplayFactory(),
                                                 "Display",
                                                 empty, empty,
                                                 &lookup_name,
                                                 &display_name );
  QApplication::restoreOverrideCursor();

  if( dialog->exec() == QDialog::Accepted )
  {
    manager_->createDisplay( lookup_name, display_name, true );
  }
  activateWindow(); // Force keyboard focus back on main window.
}

void DisplaysPanel::onDeleteDisplay()
{
  QList<Display*> displays_to_delete = property_grid_->getSelectedObjects<Display>();

  for( int i = 0; i < displays_to_delete.size(); i++ )
  {
    delete displays_to_delete[ i ];
  }
  manager_->notifyConfigChanged();
}

void DisplaysPanel::onSelectionChanged()
{
  QList<Display*> displays = property_grid_->getSelectedObjects<Display>();

  int num_displays_selected = displays.size();

  remove_button_->setEnabled( num_displays_selected > 0 );
  rename_button_->setEnabled( num_displays_selected == 1 );
}

void DisplaysPanel::onRenameDisplay()
{
  QList<Display*> displays = property_grid_->getSelectedObjects<Display>();
  if( displays.size() == 0 )
  {
    return;
  }
  Display* display_to_rename = displays[ 0 ];

  if( !display_to_rename )
  {
    return;
  }

  QString old_name = display_to_rename->getName();
  QString new_name = QInputDialog::getText( this, "Rename Display", "New Name?", QLineEdit::Normal, old_name );

  if( new_name.isEmpty() || new_name == old_name )
  {
    return;
  }

  display_to_rename->setName( new_name );
}

void DisplaysPanel::save( YAML::Emitter& emitter )
{
  tree_with_help_->save( emitter );
}

void DisplaysPanel::load( const YAML::Node& yaml_node )
{
  tree_with_help_->load( yaml_node );
}

} // namespace rviz
