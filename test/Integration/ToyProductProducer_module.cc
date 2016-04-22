//--------------------------------------------------------------------
//
// Produces an ToyProductProducer instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

using art::RangeSet;

namespace arttest {

  class ToyProductProducer : public art::EDProducer {
  public:

    explicit ToyProductProducer( fhicl::ParameterSet const& )
    {
      produces<StringProduct,art::InRun>();
      produces<StringProduct,art::InRun>("bgnRun");
      produces<StringProduct,art::InRun>("endRun");

      produces<StringProduct,art::InSubRun>();
      produces<StringProduct,art::InSubRun>("bgnSubRun");
      produces<StringProduct,art::InSubRun>("endSubRun");

      produces<StringProduct>();
    }

    ~ToyProductProducer() = default;

    void beginRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("empty"), r.fullRunRangeSet());
      r.put(std::make_unique<StringProduct>("beginRun"), "bgnRun", r.fullRunRangeSet());
    }

    void beginSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("emptyAgain"), sr.fullSubRunRangeSet());
      sr.put(std::make_unique<StringProduct>("beginSubRun"), "bgnSubRun", sr.fullSubRunRangeSet());
    }

    void produce(art::Event& e) override
    {
      e.put(std::make_unique<StringProduct>("event"));
    }

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", sr.fullSubRunRangeSet());
    }

    void endRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", r.fullRunRangeSet() );
    }

  };  // ToyProductProducer

}

DEFINE_ART_MODULE(arttest::ToyProductProducer)
