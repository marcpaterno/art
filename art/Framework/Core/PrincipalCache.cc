

//

#include "art/Framework/Core/PrincipalCache.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Utilities/Exception.h"

namespace art {

  PrincipalCache::PrincipalCache() { }

  PrincipalCache::~PrincipalCache() { }

  RunPrincipal & PrincipalCache::runPrincipal(int run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal(int run) const {
    ConstRunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  boost::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr(int run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  RunPrincipal & PrincipalCache::runPrincipal() {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal() const {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  boost::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr() {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentRunPrincipal_;
  }

  SubRunPrincipal & PrincipalCache::subRunPrincipal(int run, int subRun) {
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::subRunPrincipal(int run, int subRun) const {
    SubRunKey key(run, subRun);
    ConstSubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  boost::shared_ptr<SubRunPrincipal> PrincipalCache::subRunPrincipalPtr(int run, int subRun) {
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipalPtr\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  SubRunPrincipal & PrincipalCache::subRunPrincipal() {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentSubRunPrincipal_.get();
  }

  SubRunPrincipal const& PrincipalCache::subRunPrincipal() const {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentSubRunPrincipal_.get();
  }

  boost::shared_ptr<SubRunPrincipal> PrincipalCache::subRunPrincipalPtr() {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipalPtr\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentSubRunPrincipal_;
  }

  bool PrincipalCache::insert(boost::shared_ptr<RunPrincipal> rp) {
    int run = rp->run();
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      runPrincipals_[run] = rp;
      currentRunPrincipal_ = rp;
      return true;
    }

    iter->second->mergeRun(rp);
    currentRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::insert(boost::shared_ptr<SubRunPrincipal> srp) {
    int run = srp->run();
    int subRun = srp->subRun();
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      subRunPrincipals_[key] = srp;
      currentSubRunPrincipal_ = srp;
      return true;
    }

    iter->second->mergeSubRun(srp);
    currentSubRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::noMoreRuns() {
    return runPrincipals_.empty();
  }

  bool PrincipalCache::noMoreSubRuns() {
    return subRunPrincipals_.empty();
  }

  RunPrincipal const& PrincipalCache::lowestRun() const {
    ConstRunIterator iter = runPrincipals_.begin();
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::lowestSubRun() const {
    ConstSubRunIterator iter = subRunPrincipals_.begin();
    return *iter->second.get();
  }

  void PrincipalCache::deleteLowestRun() {
    runPrincipals_.erase(runPrincipals_.begin());
  }

  void PrincipalCache::deleteLowestSubRun() {
    subRunPrincipals_.erase(subRunPrincipals_.begin());
  }

  void PrincipalCache::deleteRun(int run) {
    runPrincipals_.erase(runPrincipals_.find(run));
  }

  void PrincipalCache::deleteSubRun(int run, int subRun) {
    subRunPrincipals_.erase(subRunPrincipals_.find(SubRunKey(run, subRun)));
  }
}
