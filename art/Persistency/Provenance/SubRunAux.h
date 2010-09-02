#ifndef DataFormats_Provenance_SubRunAux_h
#define DataFormats_Provenance_SubRunAux_h

#include <iosfwd>

#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/RunID.h"

// Auxiliary subRun data that is persistent
namespace edm
{
  struct SubRunAuxiliary;
  struct SubRunAux {
    SubRunAux() : processHistoryID_(), id_(), runID_() {}
    ~SubRunAux() {}
    mutable ProcessHistoryID processHistoryID_;
    SubRunNumber_t id_;
    RunNumber_t runID_;
  };
  void conversion(SubRunAux const& from, SubRunAuxiliary & to);
}
#endif
