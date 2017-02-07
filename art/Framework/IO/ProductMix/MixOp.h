#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h

// Template encapsulating all the attributes and functionality of a
// product mixing operation.

#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"

#include <algorithm>
#include <functional>

namespace art {
  template <typename PROD, typename OPROD> class MixOp;
}

template <typename PROD, typename OPROD>
class art::MixOp : public art::MixOpBase {
public:
  template <typename FUNC>
  MixOp(InputTag const & inputTag,
        std::string const & outputInstanceLabel,
        FUNC mixFunc,
        bool outputProduct,
        bool compactMissingProducts,
        BranchType bt);

  InputTag const & inputTag() const override;

  TypeID const & inputType() const override;

  std::string const & outputInstanceLabel() const override;

  void
  mixAndPut(Event & e,
            PtrRemapper const & remap) const override;

  void
  initializeBranchInfo(RootBranchInfoList const & rbiList) override;

  BranchID
  incomingBranchID() const override;

  BranchID
  outgoingBranchID() const override;

  void
  readFromFile(EntryNumberSequence const & seq) override;

  BranchType
  branchType() const override;

private:
  using SpecProdList = std::vector<std::shared_ptr<Wrapper<PROD> > >;

  InputTag const inputTag_;
  TypeID const inputType_;
  std::string const outputInstanceLabel_;
  MixFunc<PROD, OPROD> const mixFunc_;
  SpecProdList inProducts_;
  std::string const processName_;
  std::string const moduleLabel_;
  RootBranchInfo branchInfo_;
  bool const outputProduct_;
  bool const compactMissingProducts_;
  BranchType const branchType_;
};

template <typename PROD, typename OPROD>
template <typename FUNC>
art::MixOp<PROD, OPROD>::
MixOp(InputTag const & inputTag,
      std::string const & outputInstanceLabel,
      FUNC mixFunc,
      bool outputProduct,
      bool compactMissingProducts,
      BranchType bt)
  :
  MixOpBase(),
  inputTag_(inputTag),
  inputType_(typeid(PROD)),
  outputInstanceLabel_(outputInstanceLabel),
  mixFunc_(mixFunc),
  inProducts_(),
  processName_(ServiceHandle<TriggerNamesService>()->getProcessName()),
  moduleLabel_(ServiceHandle<CurrentModule>()->label()),
  branchInfo_(),
  outputProduct_(outputProduct),
  compactMissingProducts_(compactMissingProducts),
  branchType_(bt)
{}

template <typename PROD, typename OPROD>
art::InputTag const &
art::MixOp<PROD, OPROD>::
inputTag() const
{
  return inputTag_;
}

template <typename PROD, typename OPROD>
art::TypeID const &
art::MixOp<PROD, OPROD>::
inputType() const
{
  return inputType_;
}

template <typename PROD, typename OPROD>
std::string const &
art::MixOp<PROD, OPROD>::
outputInstanceLabel() const
{
  return outputInstanceLabel_;
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::
mixAndPut(Event & e,
          PtrRemapper const & remap) const
{
  std::unique_ptr<OPROD> rProd(new OPROD()); // Parens necessary for native types.
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    auto const endIter = inProducts_.cend();
    for (auto i = inProducts_.cbegin(); i != endIter; ++i) {
      auto const prod = (*i)->product();
      if (prod || ! compactMissingProducts_) {
        inConverted.emplace_back(prod);
      }
    }
  }
  catch (std::bad_cast const &) {
    throw Exception(errors::DataCorruption)
        << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  if (mixFunc_(inConverted, *rProd, remap)) {
    if (!outputProduct_) {
      throw Exception(errors::LogicError)
          << "Returned true (output product to be put in event) from a mix function\n"
          << "declared with outputProduct=false.\n";
    }
    if (outputInstanceLabel_.empty()) {
      e.put(std::move(rProd));
    }
    else {
      e.put(std::move(rProd), outputInstanceLabel_);
    }
  } // False means don't want this in the event.
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::
initializeBranchInfo(RootBranchInfoList const & branchInfo_List)
{
  if (!branchInfo_List.findBranchInfo(inputType_, inputTag_, branchInfo_)) {
    throw Exception(errors::ProductNotFound)
        << "Unable to find requested product "
        << inputTag_
        << " of type "
        << inputType_.friendlyClassName()
        << " in secondary input stream.\n";
  }
}

template <typename PROD, typename OPROD>
art::BranchID
art::MixOp<PROD, OPROD>::
incomingBranchID() const
{
  return BranchID(branchInfo_.branchName());
}

template <typename PROD, typename OPROD>
art::BranchID
art::MixOp<PROD, OPROD>::
outgoingBranchID() const
{
  art::BranchID result;
  if (outputProduct_) {
    TypeID const outputType(typeid(OPROD));
    BranchKey key(outputType.friendlyClassName(),
                  moduleLabel_,
                  outputInstanceLabel_,
                  processName_,
                  art::InEvent); // Outgoing product must be InEvent.
    auto I = ProductMetaData::instance().productList().find(key);
    if (I == ProductMetaData::instance().productList().end()) {
      throw Exception(errors::LogicError)
          << "MixOp unable to find branch id for a product ("
          << outputType.className()
          << ") that should have been registered!\n";
    }
    result = I->second.branchID();
  }
  return result;
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::
readFromFile(EntryNumberSequence const & seq)
{
  inProducts_.clear();
  inProducts_.reserve(seq.size());
  if (branchInfo_.branch() == 0) {
    throw Exception(errors::LogicError)
        << "Branch not initialized for read.\n";
  }
  // Make sure we don't have a ProductGetter set.
  configureRefCoreStreamer();
  // Assume the sequence is ordered per
  // MixHelper::generateEventSequence.
  auto const b = seq.cbegin(), e = seq.cend();
  for (EntryNumberSequence::const_iterator i = b; i != e; ++i) {
    auto fit = std::find(b, i, *i);
    if (fit == i) { // Need new product.
      inProducts_.emplace_back(new Wrapper<PROD>);
      Wrapper<PROD> * wp = inProducts_.back().get();
      branchInfo_.branch()->SetAddress(&wp);
      branchInfo_.branch()->GetEntry(*i);
    } else { // Already have one: find and use.
      auto pit = inProducts_.cbegin();
      std::advance(pit, std::distance(b, fit));
      inProducts_.emplace_back(*pit);
    }
  }
}

template <typename PROD, typename OPROD>
inline
art::BranchType
art::MixOp<PROD, OPROD>::
branchType() const
{
  return branchType_;
}

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
