#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h

// ======================================================================
//
// EDFilter - The base class of all "modules" used to control the flow of
// processing in a processing path.  Filters can also insert products
// into the event.  These products should be informational products about
// the filter decision.
//
// ======================================================================

#include "art/Framework/Core/EngineCreator.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"

#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art
{

  class EDFilter
    : public ProducerBase
    , public EngineCreator
  {
  public:
    static constexpr bool Pass = true;
    static constexpr bool Fail = false;

    template <typename T> friend class WorkerT;
    typedef EDFilter ModuleType;
    typedef WorkerT<EDFilter> WorkerType;

    EDFilter()
      : ProducerBase()
      , EngineCreator()
      , moduleDescription_()
      , current_context_(0)
    { }
    virtual ~EDFilter();

    template <typename PROD, BranchType B, typename TRANS>
    ProductID
    getProductID(TRANS const &translator,
                 std::string const& instanceName=std::string()) const;

    template <typename PROD, typename TRANS>
    ProductID
    getProductID(TRANS const &translator,
                 std::string const& instanceName=std::string()) const;

    // Configuration
    struct baseConfig {
      fhicl::Atom<std::string> module_type { fhicl::Key("module_type") };
    };

    template <typename T>
    struct fullConfig : baseConfig, T {}; // Multiple inheritance
                                          // allows 'module-type' key
                                          // to be listed first in
                                          // description.

    template < typename userConfig >
    class Table : public fhicl::Table<fullConfig<userConfig>> {
    public:

      Table(){}

      Table( fhicl::ParameterSet const& pset ) : Table()
      {
        std::set<std::string> const keys_to_ignore = { "module_label" };
        this->validate_ParameterSet( pset, keys_to_ignore );
        this->set_PSet( pset );
      }

    };

  protected:
    // The returned pointer will be null unless the this is currently
    // executing its event loop function ('filter').
    CurrentProcessingContext const* currentContext() const;

  private:
    bool doEvent(EventPrincipal& ep,
                  CurrentProcessingContext const* cpc);
    void doBeginJob();
    void doEndJob();
    bool doBeginRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doEndRun(RunPrincipal & rp,
                   CurrentProcessingContext const* cpc);
    bool doBeginSubRun(SubRunPrincipal & srp,
                   CurrentProcessingContext const* cpc);
    bool doEndSubRun(SubRunPrincipal & srp,
                   CurrentProcessingContext const* cpc);
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);

    std::string workerType() const {return "WorkerT<EDFilter>";}

    virtual bool filter(Event&) = 0;
    virtual void beginJob(){}
    virtual void endJob(){}
    virtual void reconfigure(fhicl::ParameterSet const&);
    virtual bool beginRun(Run &){return true;}
    virtual bool endRun(Run &){return true;}
    virtual bool beginSubRun(SubRun &){return true;}
    virtual bool endSubRun(SubRun &){return true;}
    virtual void respondToOpenInputFile(FileBlock const&) {}
    virtual void respondToCloseInputFile(FileBlock const&) {}
    virtual void respondToOpenOutputFiles(FileBlock const&) {}
    virtual void respondToCloseOutputFiles(FileBlock const&) {}

    void setModuleDescription(ModuleDescription const& md) {
      moduleDescription_ = md;
    }
    ModuleDescription moduleDescription_;
    CurrentProcessingContext const* current_context_;
  };  // EDFilter

  template <typename PROD, BranchType B, typename TRANS>
  inline
  ProductID
  EDFilter::getProductID(TRANS const &translator,
                         std::string const& instanceName) const {
    return ProducerBase::getProductID<PROD, B>(translator,
                                               moduleDescription_,
                                               instanceName);
  }

  template <typename PROD, typename TRANS>
  inline
  ProductID
  EDFilter::getProductID(TRANS const &translator,
                         std::string const& instanceName) const {
    return ProducerBase::getProductID<PROD, InEvent>(translator,
                                                     moduleDescription_,
                                                     instanceName);
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
