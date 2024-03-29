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

#ifndef RVIZ_VISUALIZATION_FRAME_H
#define RVIZ_VISUALIZATION_FRAME_H

#include <QMainWindow>
#include <QList>

#include <string>
#include <deque>
#include <set>

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/node.h>

#include "rviz/window_manager_interface.h"
#include "rviz/panel.h"
#include "rviz/pluginlib_factory.h"

class QSplashScreen;
class QAction;
class QActionGroup;
class QTimer;
class QDockWidget;
class QLabel;

namespace rviz
{

class RenderPanel;
class DisplaysPanel;
class ViewsPanel;
class TimePanel;
class SelectionPanel;
class ToolPropertiesPanel;
class VisualizationManager;
class Tool;
class HelpPanel;
class WidgetGeometryChangeDetector;

/** @brief The main rviz window.
 *
 * VisualizationFrame is a QMainWindow, which means it has a center
 * area and a bunch of dock areas around it.  The central widget here
 * is a RenderPanel, and around it (by default) are a DisplaysPanel,
 * ViewsPanel, TimePanel, SelectionPanel, and ToolPropertiesPanel.  At
 * the top is a toolbar with "Move Camera", "Select", etc.  There is
 * also a menu bar with file/open, etc.
 */
class VisualizationFrame : public QMainWindow, public WindowManagerInterface
{
Q_OBJECT
public:
  VisualizationFrame( QWidget* parent = 0 );
  ~VisualizationFrame();

  /** @brief Call this @e before initialize() to have it take effect. */
  void setShowChooseNewMaster( bool show );

  /** @brief Set the path to the help file.  Should contain HTML.
   * Default is a file in the RViz package. */
  void setHelpPath( const QString& help_path );

  /** @brief Set the path to the "splash" image file.  This image is
   * shown during initialization and loading of the first config file.
   * Default is a file in the RViz package.  To prevent splash image
   * from showing, set this to an empty string. */
  void setSplashPath( const QString& splash_path );

  void initialize( const QString& display_config_file = "" );

  VisualizationManager* getManager() { return manager_; }

  // overrides from WindowManagerInterface
  virtual QWidget* getParentWindow();
  virtual QDockWidget* addPane( const QString& name,
                                QWidget* panel,
                                Qt::DockWidgetArea area = Qt::LeftDockWidgetArea,
                                bool floating = true );

  /** @brief Load the "general" config file, which has just the few
   * things which should not be saved with a display config.
   *
   * Loads from the file named in persistent_settings_file_. */
  void loadPersistentSettings();

  /** @brief Save the "general" config file, which has just the few
   * things which should not be saved with a display config.
   *
   * Saves to the file named in persistent_settings_file_. */
  void savePersistentSettings();

  /** @brief Load display and other settings from the given file.
   * @param path The full path of the config file to load from. */
  void loadDisplayConfig( const QString& path );

  /** @brief Save display and other settings to the given file.
   * @param path The full path of the config file to save into. */
  void saveDisplayConfig( const QString& path );

public Q_SLOTS:
  /** @brief Call this to let the frame know that something that would
   *         get saved in the display config has changed. */
  void setDisplayConfigModified();

  /** Set the message displayed in the status bar */
  void setStatus( const QString & message );

Q_SIGNALS:
  /** @brief Emitted during file-loading and initialization to indicate progress. */
  void statusUpdate( const QString& message );

protected Q_SLOTS:
  void onOpen();
  void save();
  void saveAs();
  void onSaveImage();
  void onRecentConfigSelected();
  void onHelpWiki();
  void openNewPanelDialog();
  void openNewToolDialog();
  void showHelpPanel();

  /** @brief Remove a the tool whose name is given by remove_tool_menu_action->text(). */ 
  void onToolbarRemoveTool( QAction* remove_tool_menu_action );

  /** @brief Looks up the Tool for this action and calls
   * VisualizationManager::setCurrentTool(). */
  void onToolbarActionTriggered( QAction* action );

  /** @brief Add the given tool to this frame's toolbar.
   *
   * This creates a QAction internally which listens for the Tool's
   * shortcut key.  When the action is triggered by the toolbar or by
   * the shortcut key, onToolbarActionTriggered() is called. */
  void addTool( Tool* tool );

  /** @brief Remove the given tool from the frame's toolbar. */
  void removeTool( Tool* tool );

  /** @brief Mark the given tool as the current one.
   *
   * This is purely a visual change in the GUI, it does not call any
   * tool functions. */
  void indicateToolIsCurrent(Tool* tool);

  /** @brief Save the current state and quit with exit code 255 to
   * signal the wrapper that we would like to restart with a different
   * ROS master URI. */
  void changeMaster();

  /** @brief Delete a panel widget.
   *
   * The sender() of the signal should be a QAction whose text() is
   * the name of the panel. */
  void onDeletePanel();

protected Q_SLOTS:
  /** @brief Set loading_ to false. */
  void markLoadingDone();

  /** @brief Set the default directory in which to save screenshot images. */
  void setImageSaveDirectory( const QString& directory );

protected:
  /** @brief Initialize the default config directory (~/.rviz) and set
   * up the persistent_settings_file_ and display_config_file_
   * variables. */
  void initConfigs();

  void initMenus();

  /** @brief Check for unsaved changes, prompt to save config, etc.
   * @return true if it is OK to exit, false if not. */
  bool prepareToExit();

  virtual void moveEvent( QMoveEvent* event );
  virtual void closeEvent( QCloseEvent* event );

  void markRecentConfig(const std::string& path);
  void updateRecentConfigMenu();

  QRect hackedFrameGeometry();

  QDockWidget* addCustomPanel( const QString& name,
                               const QString& class_lookup_name,
                               Qt::DockWidgetArea area = Qt::LeftDockWidgetArea,
                               bool floating = true );

  /** @brief Loads custom panels from map key "Custom Panels". */
  void loadCustomPanels( const YAML::Node& yaml_node );

  /** @brief Saves custom panels to a map key "Custom Panels". */
  void saveCustomPanels( YAML::Emitter& emitter );

  void loadWindowGeometry( const YAML::Node& yaml_node );
  void saveWindowGeometry( YAML::Emitter& emitter );

  /** @brief Load the properties of all subsystems from the given yaml node.
   * 
   * This is what is called when loading a "*.rviz" file.
   *
   * @param yaml_node Must be a YAML map.
   * @sa save()
   */
  virtual void load( const YAML::Node& yaml_node );

  /**
   * \brief Save the properties of each subsystem and most editable rviz
   *        data.  Emitter must be in a map context.
   * \param emitter The yaml emitter to write to.
   * \sa loadDisplayConfig()
   */
  virtual void save( YAML::Emitter& emitter );

  /** @brief Return true if the give file is writable, false if not. */
  bool fileIsWritable( const std::string& path );
  
  /** @brief Set the display config file path.
   *
   * This does not load the given file, it just sets the member
   * variable and updates the window title. */
  void setDisplayConfigFile( const std::string& path );

  RenderPanel* render_panel_;
  DisplaysPanel* displays_panel_;
  ViewsPanel* views_panel_;
  TimePanel* time_panel_;
  SelectionPanel* selection_panel_;
  ToolPropertiesPanel* tool_properties_panel_;

  HelpPanel* help_panel_;
  QAction* show_help_action_;

  std::string config_dir_;
  std::string persistent_settings_file_;
  std::string display_config_file_;
  std::string default_display_config_file_;
  std::string last_config_dir_;
  std::string last_image_dir_;
  std::string home_dir_;

  QMenu* file_menu_;
  QMenu* recent_configs_menu_;
  QMenu* view_menu_;
  QMenu* delete_view_menu_;
  QMenu* plugins_menu_;
  QList<QAction*> view_menu_actions_;

  QToolBar* toolbar_;

  VisualizationManager* manager_;

  std::string package_path_;
  std::string help_path_;
  QString splash_path_;

  QSplashScreen* splash_;

  typedef std::deque<std::string> D_string;
  D_string recent_configs_;

  QPoint first_position_;
  QPoint position_correction_;
  int num_move_events_;
  QActionGroup* toolbar_actions_;
  std::map<QAction*,Tool*> action_to_tool_map_;
  std::map<Tool*,QAction*> tool_to_action_map_;
  bool show_choose_new_master_option_;

  PluginlibFactory<Panel>* panel_factory_;

  struct PanelRecord
  {
    Panel* panel;
    QDockWidget* dock;
    QString name;
    QString class_id;
    QAction* delete_action;
  };
  QList<PanelRecord> custom_panels_;

  QAction* add_tool_action_;
  QMenu* remove_tool_menu_;

  bool initialized_;
  WidgetGeometryChangeDetector* geom_change_detector_;
  bool loading_; ///< True just when loading a display config file, false all other times.
  QTimer* post_load_timer_; ///< Single-shot timer for calling postLoad() a short time after loadDisplayConfig() finishes.

  QLabel* status_label_;
};

}

#endif // RVIZ_VISUALIZATION_FRAME_H
