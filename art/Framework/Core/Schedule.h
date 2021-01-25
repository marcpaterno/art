#ifndef art_Framework_Core_Schedule_h
#define art_Framework_Core_Schedule_h
// vim: set sw=2 expandtab :

// ======================================================================
// Schedule
//
// A schedule contains all trigger paths and the end path executor for
// a stream of events.
//
// Processing of an event happens by pushing the event through the
// Paths. The scheduler performs the reset() on each of the workers
// independent of the Path objects.
// ======================================================================

#include "art/Framework/Core/EndPathExecutor.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/TriggerPathsExecutor.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueue.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  class ActivityRegistry;

  class Schedule {
  public:
    Schedule(ScheduleID sid,
             PathManager& pm,
             ActionTable const& actions,
             ActivityRegistry const& aReg,
             UpdateOutputCallbacks& outputCallbacks,
             std::unique_ptr<Worker> triggerResultsInserter,
             std::shared_ptr<hep::concurrency::SerialTaskQueue> end_path_queue);

    // Disable copy/move operations
    Schedule(Schedule const&) = delete;
    Schedule(Schedule&&) = delete;
    Schedule& operator=(Schedule const&) = delete;
    Schedule& operator=(Schedule&&) = delete;

    // API presented to EventProcessor
    void process(Transition, Principal&);
    void process_event_modifiers(tbb::task* endPathTask);
    void process_event_observers();
    void beginJob();
    void endJob();
    void respondToOpenInputFile(FileBlock const&);
    void respondToCloseInputFile(FileBlock const&);
    void respondToOpenOutputFiles(FileBlock const&);
    void respondToCloseOutputFiles(FileBlock const&);

    // End-path API
    bool
    outputsToOpen() const
    {
      return epExec_.outputsToOpen();
    }

    bool
    outputsToClose() const
    {
      return epExec_.outputsToClose();
    }

    void
    recordOutputClosureRequests(Granularity const granularity)
    {
      return epExec_.recordOutputClosureRequests(granularity);
    }

    bool
    someOutputsOpen() const
    {
      return epExec_.someOutputsOpen();
    }

    void
    closeAllOutputFiles()
    {
      epExec_.closeAllOutputFiles();
    }

    void
    openSomeOutputFiles(FileBlock const& fb)
    {
      epExec_.openSomeOutputFiles(fb);
    }

    void
    closeSomeOutputFiles()
    {
      epExec_.closeSomeOutputFiles();
    }

    void
    writeEvent()
    {
      assert(eventPrincipal_);
      epExec_.writeEvent(*eventPrincipal_);
      // Delete principal
      eventPrincipal_.reset();
    }

    void
    incrementInputFileNumber()
    {
      epExec_.incrementInputFileNumber();
    }

    void
    setOutputFileStatus(OutputFileStatus const ofs)
    {
      epExec_.setOutputFileStatus(ofs);
    }

    OutputFileStatus
    fileStatus() const
    {
      return epExec_.fileStatus_.load();
    }

    bool
    allAtLimit() const
    {
      return epExec_.allAtLimit();
    }

    // Run level
    void
    seedRunRangeSet(RangeSetHandler const& rsh)
    {
      epExec_.seedRunRangeSet(rsh);
    }

    void
    setRunAuxiliaryRangeSetID(RangeSet const& rs)
    {
      epExec_.setRunAuxiliaryRangeSetID(rs);
    }

    void
    writeRun(RunPrincipal& rp)
    {
      epExec_.writeRun(rp);
    }

    RangeSetHandler const&
    runRangeSetHandler()
    {
      return *epExec_.runRangeSetHandler_.get();
    }

    // SubRun level
    void
    seedSubRunRangeSet(RangeSetHandler const& rsh)
    {
      epExec_.seedSubRunRangeSet(rsh);
    }
    void
    setSubRunAuxiliaryRangeSetID(RangeSet const& rs)
    {
      epExec_.setSubRunAuxiliaryRangeSetID(rs);
    }
    void
    writeSubRun(SubRunPrincipal& srp)
    {
      epExec_.writeSubRun(srp);
    }

    RangeSetHandler const&
    subRunRangeSetHandler()
    {
      return *epExec_.subRunRangeSetHandler_.get();
    }

    void
    accept_principal(std::unique_ptr<EventPrincipal> principal)
    {
      assert(principal);
      eventPrincipal_ = std::move(principal);
    }

    EventPrincipal&
    event_principal()
    {
      assert(eventPrincipal_);
      return *eventPrincipal_;
    }

    template <typename T>
    void
    push_onto_end_path_queue(T t)
    {
      endPathQueue_->push(std::move(t));
    }

    class EndPathRunnerTask;

  private:
    ScheduleContext const context_;
    ActionTable const& actions_;
    ActivityRegistry const& actReg_;
    EndPathExecutor epExec_;
    TriggerPathsExecutor tpsExec_;

    // Shared with other schedules
    std::shared_ptr<hep::concurrency::SerialTaskQueue> endPathQueue_{};
    std::unique_ptr<EventPrincipal> eventPrincipal_{nullptr};
  };
} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_Schedule_h */
