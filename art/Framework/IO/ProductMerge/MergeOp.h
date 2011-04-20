#ifndef art_Framework_IO_ProductMerge_MergeOp_h
#define art_Framework_IO_ProductMerge_MergeOp_h

#include "art/Framework/Core/Event.h"
#include "art/Framework/IO/ProductMerge/MergeOpBase.h"
#include "art/Utilities/InputTag.h"
#include "cpp0x/functional"

namespace art {
  template <typename PROD> class MergeOp;
}

template <typename PROD>
class art::MergeOp : public art::MergeOpBase {
public:
  template <typename FUNC>
  MergeOp(InputTag const &inputTag,
          std::string const &outputInstanceLabel,
          FUNC mergeFunc);

  virtual
  InputTag const &inputTag() const;

  virtual
  TypeID const &inputType() const;

  virtual
  std::string const &outputInstanceLabel() const;

  virtual
  void
  mergeAndPut(Event &e, PtrRemapper const &remap) const;

  virtual
  void
  initProductList(size_t nSecondaries = 0);

private:
  typedef std::vector<Wrapper<PROD> > SpecProdList;

  InputTag inputTag_;
  TypeID const inputType_;
  std::string outputInstanceLabel_;
  std::function<void (std::vector<PROD const *> const &,
                      PROD &,
                      PtrRemapper const &)> mergeFunc_;

SpecProdList inProducts_;
typename SpecProdList::iterator prodIter_;
typename SpecProdList::iterator productsEnd_;
};

template <typename PROD>
template <typename FUNC>
art::
MergeOp<PROD>::MergeOp(InputTag const &inputTag,
                       std::string const &outputInstanceLabel,
                       FUNC mergeFunc)
  :
  inputTag_(inputTag),
  inputType_(typeid(PROD)),
  outputInstanceLabel_(outputInstanceLabel),
  mergeFunc_(mergeFunc),
  inProducts_(),
  prodIter_(inProducts_.begin()),
  productsEnd_(inProducts_.end())
{}

template <typename PROD>
art::InputTag const &
art::MergeOp<PROD>::
inputTag() const {
  return inputTag_;
}

template <typename PROD>
art::TypeID const &
art::MergeOp<PROD>::
inputType() const {
  return inputType_;
}

template <typename PROD>
std::string const &
art::MergeOp<PROD>::
outputInstanceLabel() const {
  return outputInstanceLabel_;
}

template <typename PROD>
void
art::MergeOp<PROD>::
mergeAndPut(Event &e,
            PtrRemapper const &remap) const {
  std::auto_ptr<PROD> rProd(new PROD);
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    for (typename SpecProdList::const_iterator
           i = inProducts_.begin(),
           endIter = inProducts_.end();
         i != endIter;
         ++i) {
      inConverted.push_back(i->product());
      if (!inConverted.back()) {
        throw Exception(errors::ProductNotFound)
          << "While processing products of type "
          << TypeID(*rProd).friendlyClassName()
          << " for merging: a secondary event was missing a product.\n";
      }
    }
  }
  catch (std::bad_cast const &) {
    throw Exception(errors::DataCorruption)
      << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  mergeFunc_(inConverted, *rProd, remap);
  if (outputInstanceLabel_.empty()) {
    e.put(rProd);
  } else {
    e.put(rProd, outputInstanceLabel_);
  }
}

template <typename PROD>
void
art::MergeOp<PROD>::
initProductList(size_t nSecondaries) {
  inProducts_.clear();
  if (nSecondaries) {
    inProducts_.reserve(nSecondaries);
    inProducts_.resize(nSecondaries);
  }
  prodIter_ = inProducts_.begin();
  productsEnd_ = inProducts_.end();
}

#endif /* art_Framework_IO_ProductMerge_MergeOp_h */

// Local Variables:
// mode: c++
// End:
