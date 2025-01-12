// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Modified by Opera Software ASA
// @copied-from chromium/src/chrome/browser/ui/webui/net_internals/source_tracker.js
// @last-synchronized b7dc66d590d1225b23736e869eb96b94add4ab2c

var SourceTracker = (function() {
  'use strict';

  /**
   * This class keeps track of all NetLog events, grouped into per-source
   * streams. It receives events from EventsTracker, and passes
   * them on to all its observers.
   *
   * @constructor
   */
  function SourceTracker() {
    assertFirstConstructorCall(SourceTracker);

    // Observers that only want to receive lists of updated SourceEntries.
    this.sourceEntryObservers_ = [];

    // True when cookies and authentication information should be removed from
    // displayed events.  When true, such information should be hidden from
    // all pages.
    this.privacyStripping_ = true;

    // True when times should be displayed as milliseconds since the first
    // event, as opposed to milliseconds since January 1, 1970.
    this.useRelativeTimes_ = true;

    this.clearEntries_();

    EventsTracker.getInstance().addLogEntryObserver(this);
  }

  cr.addSingletonGetter(SourceTracker);

  SourceTracker.prototype = {
    /**
     * Clears all log entries and SourceEntries and related state.
     */
    clearEntries_: function() {
      // Used for sorting entries with automatically assigned IDs.
      this.maxReceivedSourceId_ = 0;

      // Next unique id to be assigned to a log entry without a source.
      // Needed to identify associated GUI elements, etc.
      this.nextSourcelessEventId_ = -1;

      // Ordered list of log entries.  Needed to maintain original order when
      // generating log dumps
      this.capturedEvents_ = [];

      this.sourceEntries_ = {};
    },

    /**
     * Returns a list of all SourceEntries.
     */
    getAllSourceEntries: function() {
      return this.sourceEntries_;
    },

    /**
     * Returns the description of the specified SourceEntry, or an empty string
     * if it doesn't exist.
     */
    getDescription: function(id) {
      var entry = this.getSourceEntry(id);
      if (entry)
        return entry.getDescription();
      return '';
    },

    /**
     * Returns the specified SourceEntry.
     */
    getSourceEntry: function(id) {
      return this.sourceEntries_[id];
    },

    /**
     * Sends each entry to all observers and updates |capturedEvents_|.
     * Also assigns unique ids to log entries without a source.
     */
    onReceivedLogEntries: function(logEntries) {
      // List source entries with new log entries.  Sorted chronologically, by
      // first new log entry.
      var updatedSourceEntries = [];

      var updatedSourceEntryIdMap = {};

      for (var e = 0; e < logEntries.length; ++e) {
        var logEntry = logEntries[e];

        // Assign unique ID, if needed.
        // TODO(mmenke):  Remove this, and all other code to handle 0 source
        //                IDs when M19 hits stable.
        if (logEntry.source.id == 0) {
          logEntry.source.id = this.nextSourcelessEventId_;
          --this.nextSourcelessEventId_;
        } else if (this.maxReceivedSourceId_ < logEntry.source.id) {
          this.maxReceivedSourceId_ = logEntry.source.id;
        }

        // Create/update SourceEntry object.
        var sourceEntry = this.sourceEntries_[logEntry.source.id];
        if (!sourceEntry) {
          sourceEntry = new SourceEntry(logEntry, this.maxReceivedSourceId_);
          this.sourceEntries_[logEntry.source.id] = sourceEntry;
        } else {
          sourceEntry.update(logEntry);
        }

        // Add to updated SourceEntry list, if not already in it.
        if (!updatedSourceEntryIdMap[logEntry.source.id]) {
          updatedSourceEntryIdMap[logEntry.source.id] = sourceEntry;
          updatedSourceEntries.push(sourceEntry);
        }
      }

      this.capturedEvents_ = this.capturedEvents_.concat(logEntries);
      for (var i = 0; i < this.sourceEntryObservers_.length; ++i) {
        this.sourceEntryObservers_[i].onSourceEntriesUpdated(
            updatedSourceEntries);
      }
    },

    /**
     * Called when all log events have been deleted.
     */
    onAllLogEntriesDeleted: function() {
      this.clearEntries_();
      for (var i = 0; i < this.sourceEntryObservers_.length; ++i)
        this.sourceEntryObservers_[i].onAllSourceEntriesDeleted();
    },

    /**
     * Sets the value of |privacyStripping_| and informs log observers
     * of the change.
     */
    setPrivacyStripping: function(privacyStripping) {
      this.privacyStripping_ = privacyStripping;
      for (var i = 0; i < this.sourceEntryObservers_.length; ++i) {
        if (this.sourceEntryObservers_[i].onPrivacyStrippingChanged)
          this.sourceEntryObservers_[i].onPrivacyStrippingChanged();
      }
    },

    /**
     * Returns whether or not cookies and authentication information should be
     * displayed for events that contain them.
     */
    getPrivacyStripping: function() {
      return this.privacyStripping_;
    },

    /**
     * Sets the value of |useRelativeTimes_| and informs log observers
     * of the change.
     */
    setUseRelativeTimes: function(useRelativeTimes) {
      this.useRelativeTimes_ = useRelativeTimes;
      for (var i = 0; i < this.sourceEntryObservers_.length; ++i) {
        if (this.sourceEntryObservers_[i].onUseRelativeTimesChanged)
          this.sourceEntryObservers_[i].onUseRelativeTimesChanged();
      }
    },

    /**
     * Returns true if times should be displayed as milliseconds since the first
     * event.
     */
    getUseRelativeTimes: function() {
      return this.useRelativeTimes_;
    },

    /**
     * Adds a listener of SourceEntries. |observer| will be called back when
     * SourceEntries are added or modified, source entries are deleted, or
     * privacy stripping changes:
     *
     *   observer.onSourceEntriesUpdated(sourceEntries)
     *   observer.onAllSourceEntriesDeleted()
     *   observer.onPrivacyStrippingChanged()
     */
    addSourceEntryObserver: function(observer) {
      this.sourceEntryObservers_.push(observer);
    }
  };

  return SourceTracker;
})();
