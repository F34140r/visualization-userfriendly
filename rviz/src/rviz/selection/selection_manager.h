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

#ifndef RVIZ_SELECTION_MANAGER_H
#define RVIZ_SELECTION_MANAGER_H

#include "forwards.h"
#include "selection_handler.h"
#include "selection_args.h"
#include "rviz/properties/forwards.h"

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <OGRE/OgreTexture.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreMovableObject.h>

#include <vector>
#include <set>

//#define PICKING_DEBUG

namespace ogre_tools
{
class Object;
}

namespace Ogre
{
class SceneManager;
class Viewport;
class WireBoundingBox;
class SceneNode;
class Material;
class PixelBox;
class Rectangle2D;
class MovableObject;
}

namespace rviz
{
class ViewportMouseEvent;
class VisualizationManager;
class PropertyManager;


class SelectionManager
{
public:
  enum SelectType
  {
    Add,
    Remove,
    Replace
  };

  SelectionManager(VisualizationManager* manager);
  ~SelectionManager();

  void initialize();

  void clearHandlers();
  void addObject(CollObjectHandle obj, const SelectionHandlerPtr& handler);
  void removeObject(CollObjectHandle obj);

  void highlight(Ogre::Viewport* viewport, int x1, int y1, int x2, int y2);
  void removeHighlight();

  void select(Ogre::Viewport* viewport, int x1, int y1, int x2, int y2, SelectType type);

  CollObjectHandle createCollisionForObject(ogre_tools::Object* obj, const SelectionHandlerPtr& handler, CollObjectHandle coll = 0);
  CollObjectHandle createCollisionForEntity(Ogre::Entity* entity, const SelectionHandlerPtr& handler, CollObjectHandle coll = 0);

  void update();

  void setSelection(const M_Picked& objs);
  void addSelection(const M_Picked& objs);
  void removeSelection(const M_Picked& objs);
  const M_Picked& getSelection() { return selection_; }

  SelectionHandlerPtr getHandler(CollObjectHandle obj);

  void addPickTechnique(CollObjectHandle handle, const Ogre::MaterialPtr& material);

  inline CollObjectHandle createHandle()
  {
    if (uid_counter_ > 0x00ffffff)
    {
      uid_counter_ = 0;
    }

    uint32_t handle = 0;

    do
    {
      handle = (++uid_counter_)<<4;
      handle ^= 0x00707070;
      handle &= 0x00ffffff;
    } while (objects_.find(handle) != objects_.end());

    return handle;
  }

  void focusOnSelection();

protected:
  std::pair<Picked, bool> addSelection(const Picked& obj);
  void removeSelection(const Picked& obj);

  void setHighlightRect(Ogre::Viewport* viewport, int x1, int y1, int x2, int y2);
  void pick(Ogre::Viewport* viewport, int x1, int y1, int x2, int y2, M_Picked& results);
  void renderAndUnpack(Ogre::Viewport* viewport, uint32_t pass, int x1, int y1, int x2, int y2, V_Pixel& pixels);
  void unpackColors(Ogre::Viewport* pick_viewport, Ogre::Viewport* render_viewport, const Ogre::PixelBox& box, int x1, int y1, int x2, int y2, V_Pixel& pixels);

  VisualizationManager* vis_manager_;

  boost::recursive_mutex global_mutex_;

  typedef boost::unordered_map<CollObjectHandle, SelectionHandlerPtr> M_CollisionObjectToSelectionHandler;
  M_CollisionObjectToSelectionHandler objects_;

  bool highlight_enabled_;

  struct Highlight
  {
    int x1;
    int y1;
    int x2;
    int y2;
    Ogre::Viewport* viewport;
  };
  Highlight highlight_;

  M_Picked selection_;

  const static uint32_t s_num_render_textures_ = 2; // If you want to change this number to something > 3 you must provide more width for extra handles in the Picked structure (currently a u64)
  const static uint32_t s_render_texture_size_ = 1024;
  Ogre::TexturePtr render_textures_[s_num_render_textures_];
  Ogre::PixelBox pixel_boxes_[s_num_render_textures_];

  uint32_t uid_counter_;

  Ogre::Rectangle2D* highlight_rectangle_;
  Ogre::SceneNode* highlight_node_;

  V_Pixel pixel_buffer_;

#if defined(PICKING_DEBUG)
  Ogre::SceneNode* debug_nodes_[s_num_render_textures_];
#endif

public:
  SelectionSetSignal& getSelectionSetSignal() { return selection_set_; }
  SelectionSettingSignal& getSelectionSettingSignal() { return selection_setting_; }
  SelectionAddedSignal& getSelectionAddedSignal() { return selection_added_; }
  SelectionRemovedSignal& getSelectionRemovedSignal() { return selection_removed_; }

protected:
  SelectionSettingSignal selection_setting_;
  SelectionSetSignal selection_set_;
  SelectionAddedSignal selection_added_;
  SelectionRemovedSignal selection_removed_;
};

} // namespace rviz

#endif // RVIZ_SELECTION_MANAGER_H
