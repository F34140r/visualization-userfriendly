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

#include <yaml-cpp/node.h>
#include <yaml-cpp/emitter.h>

#include <ros/package.h>

#include "rviz/display_context.h"
#include "rviz/properties/property.h"
#include "rviz/properties/yaml_helpers.h"
#include "rviz/load_resource.h"

#include "rviz/tool.h"

namespace rviz
{

Tool::Tool()
  : property_container_( new Property() )
{
}

Tool::~Tool()
{
  delete property_container_;
}

void Tool::initialize( DisplayContext* context )
{
  context_ = context;
  scene_manager_ = context_->getSceneManager();  

  // Let subclasses do initialization if they want.
  onInitialize();
}

void Tool::setIcon( const QIcon& icon )
{
  icon_=icon;
  icon_cursor_=makeIconCursor( icon.pixmap(16), "tool_cursor:"+name_ );
}


void Tool::setName( const QString& name )
{
  name_ = name;
  property_container_->setName( name_ );
}

void Tool::setDescription( const QString& description )
{
  description_ = description;
  property_container_->setDescription( description_ );
}

void Tool::load( const YAML::Node& yaml_node )
{
  property_container_->loadChildren( yaml_node );
}

void Tool::save( YAML::Emitter& emitter )
{
  emitter << YAML::BeginMap;
  saveChildren( emitter );
  emitter << YAML::EndMap;
}

void Tool::saveChildren( YAML::Emitter& emitter )
{
  emitter << YAML::Key << "Class" << YAML::Value << getClassId();

  property_container_->saveChildren( emitter );
}

} // end namespace rviz
