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

#ifndef RVIZ_DEPTH_CLOUD_DISPLAY_H
#define RVIZ_DEPTH_CLOUD_DISPLAY_H

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <tf/message_filter.h>

#include <image_geometry/pinhole_camera_model.h>

#include "rviz/properties/enum_property.h"
#include "rviz/properties/float_property.h"
#include "rviz/properties/bool_property.h"
#include "rviz/properties/int_property.h"
#include "rviz/properties/ros_topic_property.h"

#include <rviz/display.h>

#include <rviz/default_plugin/point_cloud_common.h>

using namespace rviz;
using namespace message_filters::sync_policies;

namespace rviz
{

// Encapsulate differences between processing float and uint16_t depths
template<typename T> struct DepthTraits {};

template<>
struct DepthTraits<uint16_t>
{
  static inline bool valid(float depth) { return depth != 0.0; }
  static inline float toMeters(uint16_t depth) { return depth * 0.001f; } // originally mm
};

template<>
struct DepthTraits<float>
{
  static inline bool valid(float depth) { return std::isfinite(depth); }
  static inline float toMeters(float depth) { return depth; }
};

/**
 * \class DepthCloudDisplay
 *
 */
class DepthCloudDisplay : public rviz::Display
{
Q_OBJECT
public:
  DepthCloudDisplay();
  virtual ~DepthCloudDisplay();

  virtual void onInitialize();

  // Overrides from Display
  virtual void update(float wall_dt, float ros_dt);
  virtual void reset();

protected Q_SLOTS:
  void updateQueueSize();
  /** @brief Fill list of available and working transport options */
  void fillTransportOptionList(EnumProperty* property);
  /** @brief Update topic and resubscribe */
  virtual void updateTopic();


protected:
  void scanForTransportSubscriberPlugins();


  typedef std::vector<rviz::PointCloud::Point> V_Point;

  virtual void processMessage(const sensor_msgs::Image::ConstPtr& msg);
  virtual void processMessage(const sensor_msgs::ImageConstPtr& depth_msg, const sensor_msgs::ImageConstPtr& rgb_msg);
  void caminfoCallback( const sensor_msgs::CameraInfo::ConstPtr& msg );

  // overrides from Display
  virtual void onEnable();
  virtual void onDisable();

  virtual void fixedFrameChanged();

  void subscribe();
  void unsubscribe();

  void clear();

  uint32_t messages_received_;

  boost::mutex mutex_;

  // ROS stuff
  image_transport::ImageTransport depthmap_it_;
  boost::shared_ptr<image_transport::SubscriberFilter > depthmap_sub_;
  boost::shared_ptr<tf::MessageFilter<sensor_msgs::Image> > depthmap_tf_filter_;

  image_transport::ImageTransport rgb_it_;
  boost::shared_ptr<image_transport::SubscriberFilter > rgb_sub_;

  boost::shared_ptr<message_filters::Subscriber<sensor_msgs::CameraInfo> > cameraInfo_sub_;
  sensor_msgs::CameraInfo::ConstPtr camInfo_;
  boost::mutex camInfo_mutex_;

  typedef ApproximateTime<sensor_msgs::Image, sensor_msgs::Image> SyncPolicyDepthRGB;
  typedef message_filters::Synchronizer<SyncPolicyDepthRGB> SynchronizerDepthRGB;

  boost::shared_ptr<SynchronizerDepthRGB> syncDepthRGB_;

  IntProperty* queue_size_property_;
  u_int32_t queue_size_;

  RosTopicProperty* depth_topic_property_;
  EnumProperty* depth_transport_property_;

  RosTopicProperty* rgb_topic_property_;
  EnumProperty* rgb_transport_property_;

  PointCloudCommon* pointcloud_common_;

  std::set<std::string> transport_plugin_types_;

  // Conversion of floating point and uint16 depth images to point clouds (with color)
  template<typename T>
  void convert(const sensor_msgs::ImageConstPtr& depth_msg,
               const sensor_msgs::ImageConstPtr& color_msg,
               const sensor_msgs::CameraInfo::ConstPtr camInfo_msg,
               sensor_msgs::PointCloud2Ptr& cloud_msg);
};


} // namespace rviz

#endif //RVIZ_DEPTHMAP_TO_POINTCLOUD_DISPLAY_H
