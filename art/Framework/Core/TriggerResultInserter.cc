
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Persistency/Common/TriggerResults.h"

#include <memory>

namespace edm
{
  TriggerResultInserter::TriggerResultInserter(const ParameterSet& pset, const TrigResPtr& trptr) :
    trptr_(trptr),
    pset_id_(pset.id())
  {
    produces<TriggerResults>();
  }

  TriggerResultInserter::~TriggerResultInserter()
  {
  }

  void TriggerResultInserter::produce(edm::Event& e)
  {
    std::auto_ptr<TriggerResults>
      results(new TriggerResults(*trptr_, pset_id_));

    e.put(results);
  }
}
