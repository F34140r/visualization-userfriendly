/*
 * Copyright (c) 2012, Willow Garage, Inc.
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

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "rviz/display_context.h"
#include "rviz/frame_manager.h"
#include "rviz/ogre_helpers/shape.h"
#include "rviz/properties/color_property.h"
#include "rviz/properties/float_property.h"
#include "rviz/properties/int_property.h"
#include "rviz/properties/parse_color.h"

#include "range_display.h"

namespace rviz
{
RangeDisplay::RangeDisplay()
  : MessageFilterDisplay<sensor_msgs::Range>()
{
  color_property_ = new ColorProperty( "Color", Qt::white,
                                       "Color to draw the range.",
                                       this, SLOT( updateColorAndAlpha() ));

  alpha_property_ = new FloatProperty( "Alpha", 0.5,
                                       "Amount of transparency to apply to the range.",
                                       this, SLOT( updateColorAndAlpha() ));

  buffer_length_property_ = new IntProperty( "Buffer Length", 1,
                                             "Number of prior measurements to display.",
                                             this, SLOT( updateBufferLength() ));
  buffer_length_property_->setMin( 1 );
}

void RangeDisplay::onInitialize()
{
  MessageFilterDisplay<sensor_msgs::Range>::onInitialize();
  updateBufferLength();
  updateColorAndAlpha();
}

RangeDisplay::~RangeDisplay()
{
  for( size_t i = 0; i < cones_.size(); i++ )
  {
    delete cones_[ i ];
  }
}

void RangeDisplay::reset()
{
  MessageFilterDisplay<sensor_msgs::Range>::reset();
  updateBufferLength();
}

void RangeDisplay::updateColorAndAlpha()
{
  Ogre::ColourValue oc = qtToOgre( color_property_->getColor() );
  float alpha = alpha_property_->getFloat();
  for( size_t i = 0; i < cones_.size(); i++ )
  {
    cones_[i]->setColor( oc.r, oc.g, oc.b, alpha );
  }
  context_->queueRender();
}

void RangeDisplay::updateBufferLength()
{
  int buffer_length = buffer_length_property_->getInt();
  QColor color = color_property_->getColor();

  for( size_t i = 0; i < cones_.size(); i++ )
  {
    delete cones_[i];
  }
  cones_.resize( buffer_length );
  for( size_t i = 0; i < cones_.size(); i++ )
  {
    Shape* cone = new Shape( Shape::Cone, context_->getSceneManager(), scene_node_ );
    cones_[ i ] = cone;    

    Ogre::Vector3 position;
    Ogre::Quaternion orientation;
    geometry_msgs::Pose pose;
    pose.orientation.w = 1;
    Ogre::Vector3 scale( 0, 0, 0 );
    cone->setScale( scale );
    cone->setColor( color.redF(), color.greenF(), color.blueF(), 0 );
  }
}

void RangeDisplay::processMessage( const sensor_msgs::Range::ConstPtr& msg )
{
  Shape* cone = cones_[ messages_received_ % buffer_length_property_->getInt() ];

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  geometry_msgs::Pose pose;
  pose.position.x = msg->range/2 - .008824 * msg->range; // .008824 fudge factor measured, must be inaccuracy of cone model.
  pose.orientation.z = 0.707;
  pose.orientation.w = 0.707;
  if( !context_->getFrameManager()->transform( msg->header.frame_id, msg->header.stamp, pose, position, orientation ))
  {
    ROS_DEBUG( "Error transforming from frame '%s' to frame '%s'",
               msg->header.frame_id.c_str(), qPrintable( fixed_frame_ ));
  }

  cone->setPosition( position );
  cone->setOrientation( orientation );

  double cone_width = 2.0 * msg->range * tan( msg->field_of_view / 2.0 );
  Ogre::Vector3 scale( cone_width, msg->range, cone_width );
  cone->setScale( scale );

  QColor color = color_property_->getColor();
  cone->setColor( color.redF(), color.greenF(), color.blueF(), alpha_property_->getFloat() );
}

} // namespace rviz

#include <pluginlib/class_list_macros.h>
PLUGINLIB_DECLARE_CLASS( rviz, Range, rviz::RangeDisplay, rviz::Display )
