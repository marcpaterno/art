#include "art/Framework/Principal/Consumer.h"

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

namespace art {

  ProductInfo::~ProductInfo() = default;

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid)
    : consumableType_{consumableType}, typeID_{tid}
  {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid,
                           string const& label,
                           string const& instance,
                           ProcessTag const& process)
    : consumableType_{consumableType}
    , typeID_{tid}
    , label_{label}
    , instance_{instance}
    , process_{process}
  {}

  bool
  operator<(ProductInfo const& a, ProductInfo const& b)
  {
    return tie(a.consumableType_,
               a.typeID_,
               a.label_,
               a.instance_,
               a.process_.name()) < tie(b.consumableType_,
                                        b.typeID_,
                                        b.label_,
                                        b.instance_,
                                        b.process_.name());
  }

  ostream&
  operator<<(ostream& os, ProductInfo::ConsumableType const ct)
  {
    switch (ct) {
      case ProductInfo::ConsumableType::Product:
        os << "Product";
        break;
      case ProductInfo::ConsumableType::ViewElement:
        os << "ViewElement";
        break;
      case ProductInfo::ConsumableType::Many:
        os << "Many";
        break;
    }
    return os;
  }

  ostream&
  operator<<(ostream& os, ProductInfo const& info)
  {
    os << "Consumable type: " << info.consumableType_ << '\n'
       << "TypeID: " << info.typeID_ << '\n'
       << "Module label: " << info.label_ << '\n'
       << "Instance name: " << info.instance_ << '\n'
       << "Process name: " << info.process_.name() << '\n';
    return os;
  }

  ConsumesInfo::~ConsumesInfo() = default;

  ConsumesInfo::ConsumesInfo() { requireConsumes_ = false; }

  ConsumesInfo*
  ConsumesInfo::instance()
  {
    static ConsumesInfo me;
    return &me;
  }

  string
  ConsumesInfo::assemble_consumes_statement(BranchType const bt,
                                            ProductInfo const& pi)
  {
    string result;
    // Create "consumes" prefix
    switch (pi.consumableType_) {
      case ProductInfo::ConsumableType::Product:
        result += "consumes";
        break;
      case ProductInfo::ConsumableType::ViewElement:
        result += "consumesView";
        break;
      case ProductInfo::ConsumableType::Many:
        result += "consumesMany";
        break;
    }
    // .. now time for the template arguments
    result += '<';
    result += pi.typeID_.className();
    if (bt != InEvent) {
      result += ", In";
      result += BranchTypeToString(bt);
    }
    result += '>';
    // Form "(...);" string with appropriate arguments.
    result += '(';
    // Early bail out for consumesMany.
    if (pi.consumableType_ == ProductInfo::ConsumableType::Many) {
      result += ");";
      return result;
    }
    result += '"';
    result += pi.label_;
    // If the process name is non-empty, then all InputTag fields are
    // required (e.g.):
    //   "myLabel::myProcess"
    //   "myLabel::myInstance::myProcess"
    if (!pi.process_.name().empty()) {
      result += ':';
      result += pi.instance_;
      result += ':';
      result += pi.process_.name();
    } else if (!pi.instance_.empty()) {
      result += ':';
      result += pi.instance_;
    }
    result += "\");";
    return result;
  }

  string
  ConsumesInfo::module_context(ModuleDescription const& md)
  {
    string result{"module label: '"};
    result += md.moduleLabel();
    result += "' of class type '";
    result += md.moduleName();
    result += '\'';
    return result;
  }

  void
  ConsumesInfo::setRequireConsumes(bool const val)
  {
    requireConsumes_ = val;
  }

  void
  ConsumesInfo::collectConsumes(
    string const& module_label,
    array<vector<ProductInfo>, NumBranchTypes> const& consumables)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    consumables_.emplace(module_label, consumables);
  }

  void
  ConsumesInfo::validateConsumedProduct(BranchType const bt,
                                        ModuleDescription const& md,
                                        ProductInfo const& productInfo)
  {

    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (cet::binary_search_all(consumables_[md.moduleLabel()][bt],
                               productInfo)) {
      // Found it, everything is ok.
      return;
    }
    if (requireConsumes_.load()) {
      throw Exception(errors::ProductRegistrationFailure,
                      "Consumer: an error occurred during validation of a "
                      "retrieved product\n\n")
        << "The following consumes (or mayConsume) statement is missing from\n"
        << module_context(md) << ":\n\n"
        << "  " << assemble_consumes_statement(bt, productInfo) << "\n\n";
    }
    missingConsumes_[md.moduleLabel()][bt].insert(productInfo);
  }

  void
  ConsumesInfo::showMissingConsumes() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto const& modLabelAndarySetPI : missingConsumes_) {
      auto const& modLabel = modLabelAndarySetPI.first;
      auto const& arySetPI = modLabelAndarySetPI.second;
      constexpr cet::HorizontalRule rule{60};
      mf::LogPrint log{"MTdiagnostics"};
      log << '\n'
          << rule('=') << '\n'
          << "The following consumes (or mayConsume) statements are missing "
             "from\n"
          << "module label: " << modLabel << '\n'
          << rule('-') << '\n';
      size_t i = 0;
      for (auto const& setPI : arySetPI) {
        for (auto const& pi : setPI) {
          log << "  "
              << assemble_consumes_statement(static_cast<BranchType>(i), pi)
              << '\n';
        }
        ++i;
      }
      log << rule('=');
    }
  }

} // namespace art
