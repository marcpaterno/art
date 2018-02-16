#ifndef art_Framework_Core_ProductRegistryHelper_h
#define art_Framework_Core_ProductRegistryHelper_h
// vim: set sw=2 expandtab :

//
// This class provides the produces() and reconstitutes()
// function templates used by modules to register what
// products they create or read in respectively.
//
// The constructors of an EDProducer or an EDFilter should call
// produces() for each product inserted into a principal.
// Instance names should be provided only when the module
// makes more than one instance of the same product per event.
//
// The constructors of an InputSource should call reconstitutes()
// for each product if and only if it does not update the
// UpdateOutputCallbacks with a product list.
//

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Persistable.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"

#include <array>
#include <map>
#include <memory>
#include <string>

namespace art {

  class ModuleDescription;

  namespace {

    inline void
    verifyFriendlyClassName(std::string const& fcn)
    {
      std::string errMsg;
      if (!detail::checkFriendlyName(fcn, errMsg)) {
        throw Exception(errors::Configuration)
          << errMsg
          << "In particular, underscores are not permissible anywhere in the "
             "fully-scoped\n"
             "class name, including namespaces.\n";
      }
    }

    inline void
    verifyModuleLabel(std::string const& ml)
    {
      std::string errMsg;
      if (!detail::checkModuleLabel(ml, errMsg)) {
        throw Exception(errors::Configuration) << errMsg;
      }
    }

    inline void
    verifyInstanceName(std::string const& instanceName)
    {
      std::string errMsg;
      if (!detail::checkInstanceName(instanceName, errMsg)) {
        throw Exception(errors::Configuration) << errMsg;
      }
    }

  } // unnamed namespace

  class ProductRegistryHelper {
  public: // MEMBER FUNCTIONS
    virtual ~ProductRegistryHelper();

    ProductRegistryHelper();

    ProductRegistryHelper(ProductRegistryHelper const&) = delete;
    ProductRegistryHelper(ProductRegistryHelper&&) = delete;
    ProductRegistryHelper& operator=(ProductRegistryHelper const&) = delete;
    ProductRegistryHelper& operator=(ProductRegistryHelper&&) = delete;

  public: // MEMBER FUNCTIONS

    // Used by an input source to provide a product list to be merged
    // into the set of products that will be registered.
    void
    productList(std::unique_ptr<ProductList> p)
    {
      productList_ = move(p);
    }

    void registerProducts(ProductDescriptions& productsToRegister,
                          ModuleDescription const& md);

    void fillDescriptions(ModuleDescription const& md);

    // Record the production of an object of type P, with optional
    // instance name, in the Event (by default), Run, or SubRun.
    template <typename P, BranchType B = InEvent>
    void produces(std::string const& instanceName = {},
                  Persistable const persistable = Persistable::Yes);

    // Record the reconstitution of an object of type P, in either the
    // Run, SubRun, or Event, recording that this object was
    // originally created by a module with label modLabel, and with an
    // optional instance name.
    template <typename P, BranchType B>
    TypeLabel const& reconstitutes(std::string const& modLabel,
                                   std::string const& instanceName = {});

    template <BranchType B>
    TypeLabelLookup_t const& expectedProducts() const;

  private: // MEMBER FUNCTIONS
    TypeLabel const& insertOrThrow(BranchType const bt, TypeLabel const& tl);

  private: // MEMBER DATA
    std::unique_ptr<ProductList const> productList_{nullptr};
    std::array<TypeLabelLookup_t, NumBranchTypes> typeLabelList_{{}};
  };

  template <BranchType B>
  inline TypeLabelLookup_t const&
  ProductRegistryHelper::expectedProducts() const
  {
    return typeLabelList_[B];
  }

  template <typename P, art::BranchType B>
  inline void
  ProductRegistryHelper::produces(std::string const& instanceName,
                                  Persistable const persistable)
  {
    verifyInstanceName(instanceName);
    TypeID const productType{typeid(P)};
    verifyFriendlyClassName(productType.friendlyClassName());
    bool const isTransient = (persistable == Persistable::No);
    TypeLabel const typeLabel{
      productType, instanceName, SupportsView<P>::value, isTransient};
    insertOrThrow(B, typeLabel);
  }

  template <typename P, BranchType B>
  TypeLabel const&
  ProductRegistryHelper::reconstitutes(std::string const& emulatedModule,
                                       std::string const& instanceName)
  {
    verifyModuleLabel(emulatedModule);
    verifyInstanceName(instanceName);
    TypeID const productType{typeid(P)};
    verifyFriendlyClassName(productType.friendlyClassName());
    TypeLabel const typeLabel{
      productType, instanceName, SupportsView<P>::value, emulatedModule};
    return insertOrThrow(B, typeLabel);
  }

} // namespace art

#endif /* art_Framework_Core_ProductRegistryHelper_h */

// Local Variables:
// mode: c++
// End:
