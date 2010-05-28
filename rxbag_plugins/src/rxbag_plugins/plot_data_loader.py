# Software License Agreement (BSD License)
#
# Copyright (c) 2009, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

PKG = 'rxbag_plugins'
import roslib; roslib.load_manifest(PKG)
import rospy

import bisect
import collections
import sys
import threading
import time

class PlotDataLoader(threading.Thread):
    def __init__(self, timeline, topic):
        threading.Thread.__init__(self, target=self._run)

        self._timeline = timeline
        self._topic    = topic

        self._start_stamp = self._timeline.start_stamp
        self._end_stamp   = self._timeline.end_stamp
        self._paths       = []
        self._dirty       = True

        self._entries = []
        self._loaded  = set()
        self._data    = {}

        self._listeners = []

        self._stop_flag = False
        self.setDaemon(True)
        self.start()

    # listeners

    def add_listener(self, listener):
        self._listeners.append(listener)

    def remove_listener(self, listener):
        self._listeners.remove(listener)

    # property: start_stamp

    def _get_start_stamp(self): return self._start_stamp
    
    def _set_start_stamp(self, start_stamp):
        if start_stamp == self._start_stamp:
            return

        self._start_stamp = start_stamp
        self._dirty = True
        
    start_stamp = property(_get_start_stamp, _set_start_stamp)

    # property: end_stamp

    def _get_end_stamp(self): return self._end_stamp
    
    def _set_end_stamp(self, end_stamp):
        if end_stamp == self._end_stamp:
            return
        
        self._end_stamp = end_stamp
        self._dirty = True
        
    end_stamp = property(_get_end_stamp, _set_end_stamp)

    # property: paths
        
    def _get_paths(self): return self._paths
    
    def _set_paths(self, paths):
        if set(paths) == set(self._paths):
            return

        self._paths = paths
        self._dirty = True

    paths = property(_get_paths, _set_paths)

    def stop(self):
        self._stop_flag = True
        self.join()

    ##

    def _run(self):
        while not self._stop_flag:
            if self._dirty:
                self._entries = list(self._timeline.get_entries_with_bags(self._topic, self._start_stamp, self._end_stamp))
                self._loaded  = set()
                subdivider = _subdivide(0, len(self._entries) - 1)
                self._dirty = False

            if subdivider is None or len(self._paths) == 0:
                print 'sleeping...'
                time.sleep(0.2)
                continue

            try:
                index = subdivider.next()
            except StopIteration:
                subdivider = None

            if index in self._loaded:
                continue

            with self._timeline._bag_lock:
                bag, entry = self._entries[index]
                topic, msg, msg_stamp = self._timeline.read_message(bag, entry.position)

            if not msg:
                continue

            use_header_stamp = False

            for path in self._paths:
                if use_header_stamp:
                    if msg.__class__._has_header:
                        header = msg.header
                    else:
                        header = _get_header(msg, plot_path)
                    
                    plot_stamp = header.stamp
                else:
                    plot_stamp = msg_stamp

                try:
                    y = eval('msg.' + path)
                except Exception:
                    continue

                x = (plot_stamp - self._timeline.start_stamp).to_sec()

                if path not in self._data:
                    self._data[path] = []

                bisect.insort_right(self._data[path], (x, y))

            self._loaded.add(index)

            for listener in self._listeners:
                listener()

def _get_header(msg, path):
    fields = path.split('.')
    if len(fields) <= 1:
        return None

    parent_path = '.'.join(fields[:-1])

    parent = eval('msg.' + parent_path)
    
    for slot in parent.__slots__:
        subobj = getattr(parent, slot)
        if subobj is not None and hasattr(subobj, '_type') and getattr(subobj, '_type') == 'roslib/Header':
            return subobj

    return _get_header(msg, parent_path)

def _subdivide(left, right):
    yield left
    yield right

    intervals = collections.deque([(left, right)])
    while True:
        try:
            (left, right) = intervals.popleft()
        except Exception:
            break

        mid = (left + right) / 2

        if right - left <= 1:
            continue

        yield mid

        if right - left <= 2:
            continue

        intervals.append((left, mid))
        intervals.append((mid,  right))
