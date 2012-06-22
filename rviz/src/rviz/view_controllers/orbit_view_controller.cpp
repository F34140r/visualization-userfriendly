/*
 * Copyright (c) 2009, Willow Garage, Inc.
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

#include <stdint.h>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreViewport.h>

#include "rviz/display_context.h"
#include "rviz/ogre_helpers/shape.h"
#include "rviz/properties/float_property.h"
#include "rviz/uniform_string_stream.h"
#include "rviz/viewport_mouse_event.h"

#include "rviz/view_controllers/orbit_view_controller.h"

static const float PITCH_START = Ogre::Math::HALF_PI / 2.0;
static const float YAW_START = Ogre::Math::HALF_PI * 0.5;
static const float DISTANCE_START = 10;

namespace rviz
{

OrbitViewController::OrbitViewController(DisplayContext* context, const std::string& name, Ogre::SceneNode* target_scene_node)
: ViewController(context, name, target_scene_node)
{
  focal_shape_ = new Shape(Shape::Sphere, context_->getSceneManager(), target_scene_node_);
  focal_shape_->setScale(Ogre::Vector3(0.05f, 0.05f, 0.01f));
  focal_shape_->setColor(1.0f, 1.0f, 0.0f, 0.5f);
  focal_shape_->getRootNode()->setVisible(false);

  distance_property_ = new FloatProperty( "Distance", DISTANCE_START, "Distance from the focal point.", this );
  distance_property_->setMin( 0.01 );

  yaw_property_ = new FloatProperty( "Yaw", YAW_START, "Rotation of the camera around the Z (up) axis.", this );

  pitch_property_ = new FloatProperty( "Pitch", PITCH_START, "How much the camera is tipped downward.", this );
  pitch_property_->setMax( Ogre::Math::HALF_PI - 0.001 );
  pitch_property_->setMin( -pitch_property_->getMax() );

  reset();
}

OrbitViewController::~OrbitViewController()
{
  delete focal_shape_;
}

void OrbitViewController::reset()
{
  dragging_ = false;
  focal_point_ = Ogre::Vector3::ZERO;
  yaw_property_->setFloat( YAW_START );
  pitch_property_->setFloat( PITCH_START );
  distance_property_->setFloat( DISTANCE_START );
  emitConfigChanged();
}

void OrbitViewController::updateDistance()
{
  
}

void OrbitViewController::handleMouseEvent(ViewportMouseEvent& event)
{
  float distance = distance_property_->getFloat();

  bool moved = false;
  if( event.type == QEvent::MouseButtonPress )
  {
    focal_shape_->getRootNode()->setVisible(true);
    moved = true;
    dragging_ = true;
  }
  else if( event.type == QEvent::MouseButtonRelease )
  {
    focal_shape_->getRootNode()->setVisible(false);
    moved = true;
    dragging_ = false;
  }
  else if( dragging_ && event.type == QEvent::MouseMove )
  {
    int32_t diff_x = event.x - event.last_x;
    int32_t diff_y = event.y - event.last_y;

    if( diff_x != 0 || diff_y != 0 )
    {
      // regular left-button drag
      if( event.left() && !event.shift() )
      {
        yaw( diff_x*0.005 );
        pitch( -diff_y*0.005 );
      }
      // middle or shift-left drag
      else if( event.middle() || (event.shift() && event.left()) )
      {
        float fovY = camera_->getFOVy().valueRadians();
        float fovX = 2.0f * atan( tan( fovY / 2.0f ) * camera_->getAspectRatio() );

        int width = camera_->getViewport()->getActualWidth();
        int height = camera_->getViewport()->getActualHeight();

        move( -((float)diff_x / (float)width) * distance * tan( fovX / 2.0f ) * 2.0f,
              ((float)diff_y / (float)height) * distance * tan( fovY / 2.0f ) * 2.0f,
              0.0f );
      }
      else if( event.right() )
      {
        if( event.shift() )
        {
          move(0.0f, 0.0f, diff_y * 0.1 * (distance / 10.0f));
        }
        else
        {
          zoom( -diff_y * 0.1 * (distance / 10.0f) );
        }
      }

      moved = true;
    }
  }

  if( event.wheel_delta != 0 )
  {
    int diff = event.wheel_delta;
    if( event.shift() )
    {
      move( 0, 0, -diff * 0.001 * distance );
    }
    else
    {
      zoom( diff * 0.001 * distance );
    }

    moved = true;
  }

  if( moved )
  {
    context_->queueRender();
  }
}

void OrbitViewController::onActivate()
{
  if (camera_->getProjectionType() == Ogre::PT_ORTHOGRAPHIC)
  {
    camera_->setProjectionType(Ogre::PT_PERSPECTIVE);
  }
  else
  {
    Ogre::Vector3 position = camera_->getPosition();
    Ogre::Quaternion orientation = camera_->getOrientation();

    // Determine the distance from here to the reference frame, and use that as the distance our focal point should be at
    distance_property_->setFloat( position.length() );

    Ogre::Vector3 direction = orientation * (Ogre::Vector3::NEGATIVE_UNIT_Z * distance_property_->getFloat() );
    focal_point_ = position + direction;

    calculatePitchYawFromPosition( position );
  }
}

void OrbitViewController::onDeactivate()
{
  focal_shape_->getRootNode()->setVisible(false);
  camera_->setFixedYawAxis(false);
}

void OrbitViewController::onUpdate(float dt, float ros_dt)
{
  updateCamera();
}

void OrbitViewController::lookAt( const Ogre::Vector3& point )
{
  Ogre::Vector3 camera_position = camera_->getPosition();
  focal_point_ = target_scene_node_->getOrientation().Inverse() * (point - target_scene_node_->getPosition());
  distance_property_->setFloat( focal_point_.distance( camera_position ));

  calculatePitchYawFromPosition(camera_position);
}

void OrbitViewController::onTargetFrameChanged(const Ogre::Vector3& old_reference_position, const Ogre::Quaternion& old_reference_orientation)
{
  focal_point_ += old_reference_position - reference_position_;
}

float OrbitViewController::mapAngleTo0_2Pi( float angle )
{
  angle = fmod( angle, Ogre::Math::TWO_PI );

  if( angle < 0.0f )
  {
    angle = Ogre::Math::TWO_PI + angle;
  }
  return angle;
}

void OrbitViewController::updateCamera()
{
  float distance = distance_property_->getFloat();
  float yaw = yaw_property_->getFloat();
  float pitch = pitch_property_->getFloat();

  float x = distance * cos( yaw ) * cos( pitch ) + focal_point_.x;
  float y = distance * sin( yaw ) * cos( pitch ) + focal_point_.y;
  float z = distance *               sin( pitch ) + focal_point_.z;

  Ogre::Vector3 pos( x, y, z );

  camera_->setPosition(pos);
  camera_->setFixedYawAxis(true, target_scene_node_->getOrientation() * Ogre::Vector3::UNIT_Z);
  camera_->setDirection(target_scene_node_->getOrientation() * (focal_point_ - pos));

//  ROS_INFO_STREAM( camera_->getDerivedDirection() );

  focal_shape_->setPosition(focal_point_);
}

void OrbitViewController::yaw( float angle )
{
  yaw_property_->setFloat( mapAngleTo0_2Pi( yaw_property_->getFloat() - angle ));
}

void OrbitViewController::pitch( float angle )
{
  pitch_property_->add( -angle );
}

void OrbitViewController::calculatePitchYawFromPosition( const Ogre::Vector3& position )
{
  float x = position.x - focal_point_.x;
  float y = position.y - focal_point_.y;
  float z = position.z - focal_point_.z;

  pitch_property_->setFloat( asin( z / distance_property_->getFloat() ));

  yaw_property_->setFloat( atan2( y, x ));
}

void OrbitViewController::zoom( float amount )
{
  distance_property_->add( -amount );
}

void OrbitViewController::move( float x, float y, float z )
{
  focal_point_ += camera_->getOrientation() * Ogre::Vector3( x, y, z );
  emitConfigChanged();
}

void OrbitViewController::fromString(const std::string& str)
{
}

std::string OrbitViewController::toString()
{
  return "";
}

} // end namespace rviz
