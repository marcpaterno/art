#include "art/Persistency/Common/CopyPolicy.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Common/RangeMap.h"
#include "art/Persistency/Common/VectorHolder.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/DataFrame.h"
#include "art/Persistency/Common/DataFrameContainer.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/BoolCache.h"
#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Persistency/Common/ValueMap.h"

#include <vector>

namespace {
  struct dictionary {
    edm::Wrapper<edm::DataFrameContainer> dummywdfc;
    edm::Wrapper<edm::HLTPathStatus> dummyx16;
    edm::Wrapper<std::vector<edm::HLTPathStatus> > dummyx17;
    edm::Wrapper<edm::HLTGlobalStatus> dummyx18;
    edm::Wrapper<edm::TriggerResults> dummyx19;

    edm::reftobase::IndirectVectorHolder<int> dummyx21_1;


    edm::RangeMap<int, std::vector<float>, edm::CopyPolicy<float> > dummyRangeMap1;


    std::pair<edm::ProductID, unsigned int> ppui1;
    edm::Wrapper<edm::ValueMap<int> > wvm1;
    edm::Wrapper<edm::ValueMap<unsigned int> > wvm2;
    edm::Wrapper<edm::ValueMap<bool> > wvm3;
    edm::Wrapper<edm::ValueMap<float> > wvm4;
    edm::Wrapper<edm::ValueMap<double> > wvm5;
  };
}
