#ifndef FWCore_Framework_WorkerMaker_h
#define FWCore_Framework_WorkerMaker_h

#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Framework/Core/WorkerParams.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/ParameterSet/ParameterSetDescription.h"
#include "art/Utilities/EDMException.h"

#include <memory>
#include <string>
#include "sigc++/signal.h"


namespace edm {

  class Maker {
  public:
    virtual ~Maker();
    virtual std::auto_ptr<Worker> makeWorker(WorkerParams const&,
                                             sigc::signal<void, ModuleDescription const&>& iPre,
                                             sigc::signal<void, ModuleDescription const&>& iPost) const = 0;
  protected:
    ModuleDescription createModuleDescription(WorkerParams const &p) const;
    void throwConfigurationException(ModuleDescription const &md, sigc::signal<void, ModuleDescription const&>& post, cms::Exception const& iException) const;
  };

  template <class T>
  class WorkerMaker : public Maker {
  public:
    //typedef T worker_type;
    explicit WorkerMaker();
    virtual std::auto_ptr<Worker> makeWorker(WorkerParams const&,
                                     sigc::signal<void, ModuleDescription const&>&,
                                     sigc::signal<void, ModuleDescription const&>&) const;
  };

  template <class T>
  WorkerMaker<T>::WorkerMaker() {
  }

  template <class T>
  std::auto_ptr<Worker> WorkerMaker<T>::makeWorker(WorkerParams const& p,
                                                   sigc::signal<void, ModuleDescription const&>& pre,
                                                   sigc::signal<void, ModuleDescription const&>& post) const {
    typedef T UserType;
    typedef typename UserType::ModuleType ModuleType;
    typedef typename UserType::WorkerType WorkerType;

    ModuleDescription md = createModuleDescription(p);

    std::auto_ptr<Worker> worker;
    try {
       pre(md);

       ParameterSetDescription psetDescription;
       UserType::fillDescription(psetDescription, md.moduleLabel());
       psetDescription.validate(*p.pset_);

       std::auto_ptr<ModuleType> module(WorkerType::template makeModule<UserType>(md, *p.pset_));
       worker=std::auto_ptr<Worker>(new WorkerType(module, md, p));
       post(md);
    } catch( cms::Exception& iException){
       throwConfigurationException(md, post, iException);
    }
    return worker;
  }
}

#endif
