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

#ifndef RVIZ_INTERACTIVE_MARKER_DISPLAY_H
#define RVIZ_INTERACTIVE_MARKER_DISPLAY_H

#include "rviz/default_plugin/interactive_markers/interactive_marker.h"

#include "rviz/display.h"
#include "rviz/selection/forwards.h"
#include "rviz/properties/forwards.h"

#include <map>
#include <set>

#include <visualization_msgs/InteractiveMarker.h>
#include <visualization_msgs/InteractiveMarkerArray.h>

#include <message_filters/subscriber.h>
#include <tf/message_filter.h>

namespace Ogre
{
class SceneManager;
class SceneNode;
}

namespace ogre_tools
{
class Object;
}

namespace rviz
{

class MarkerSelectionHandler;
typedef boost::shared_ptr<MarkerSelectionHandler> MarkerSelectionHandlerPtr;

class MarkerBase;
typedef boost::shared_ptr<MarkerBase> MarkerBasePtr;

typedef std::pair<std::string, int32_t> MarkerID;

/**
 * \class InteractiveMarkerDisplay
 * \brief Displays "markers" sent in by other ROS nodes on the "visualization_marker" topic
 *
 * Markers come in as visualization_msgs::Marker messages.  See the Marker message for more information.
 */
class InteractiveMarkerDisplay : public Display
{
public:
  InteractiveMarkerDisplay( const std::string& name, VisualizationManager* manager );
  virtual ~InteractiveMarkerDisplay();

  virtual void update(float wall_dt, float ros_dt);

  virtual void targetFrameChanged();
  virtual void fixedFrameChanged();
  virtual void reset();

  void setMarkerTopic(const std::string& topic);
  const std::string& getMarkerTopic() { return marker_topic_; }

  void setMarkerArrayTopic(const std::string& topic);
  const std::string& getMarkerArrayTopic() { return marker_array_topic_; }

  virtual void createProperties();

protected:

  virtual void onEnable();
  virtual void onDisable();

  // Subscribe to all message topics
  void subscribe();

  // Unsubscribe from all message topics
  void unsubscribe();

  // Processes a marker message
  void processMessage( const visualization_msgs::InteractiveMarker::ConstPtr& message );

  // ROS callback notifying us of a new marker
  void incomingMarker(const visualization_msgs::InteractiveMarker::ConstPtr& marker);

  void incomingMarkerArray(const visualization_msgs::InteractiveMarkerArray::ConstPtr& array);

  // ROS callback for failed marker receival
  void failedMarker(const visualization_msgs::InteractiveMarker::ConstPtr& marker, tf::FilterFailureReason reason);

  typedef std::vector<visualization_msgs::InteractiveMarker::ConstPtr> V_InteractiveMarkerMessage;

  // messages are placed here before being processed in update()
  typedef std::vector<visualization_msgs::InteractiveMarker::ConstPtr> V_MarkerMessage;
  V_MarkerMessage message_queue_;
  boost::mutex queue_mutex_;

  // Scene node all the marker objects are attached to
  Ogre::SceneNode* scene_node_;

  message_filters::Subscriber<visualization_msgs::InteractiveMarker> marker_sub_;
  tf::MessageFilter<visualization_msgs::InteractiveMarker> tf_filter_;

  ros::Subscriber marker_array_sub_;

  std::string marker_topic_;
  ROSTopicStringPropertyWPtr marker_topic_property_;

  std::string marker_array_topic_;
  ROSTopicStringPropertyWPtr marker_array_topic_property_;

  typedef boost::shared_ptr<InteractiveMarker> InteractiveMarkerPtr;
  typedef std::map< std::string, InteractiveMarkerPtr > M_StringToInteractiveMarkerPtr;
  M_StringToInteractiveMarkerPtr interactive_markers_;
};

} // namespace rviz

#endif /* RVIZ_MARKER_DISPLAY_H */
