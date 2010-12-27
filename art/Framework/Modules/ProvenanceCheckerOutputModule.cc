// ======================================================================
//
// ProvenanceCheckerOutputModule - Checks the consistency of provenance
//                                 stored in the framework
//
// ======================================================================

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

   class ProvenanceCheckerOutputModule
     : public OutputModule
   {
   public:
      // We do not take ownership of passed stream.  (Huh? -- WEB)
      explicit ProvenanceCheckerOutputModule(fhicl::ParameterSet const&);
      virtual ~ProvenanceCheckerOutputModule();

   private:
      virtual void write(EventPrincipal const& e);
      virtual void writeSubRun(SubRunPrincipal const&){}
      virtual void writeRun(RunPrincipal const&){}
   };  // ProvenanceCheckerOutputModule

//
// constructors and destructor
//
   ProvenanceCheckerOutputModule::ProvenanceCheckerOutputModule(fhicl::ParameterSet const& pset) :
   OutputModule(pset)
   { }

   ProvenanceCheckerOutputModule::~ProvenanceCheckerOutputModule()
   { }

//
// member functions
//
   static void markAncestors(const ProductProvenance& iInfo,
                             const BranchMapper& iMapper,
                             std::map<BranchID,bool>& oMap,
                             std::set<BranchID>& oMapperMissing) {
      for(std::vector<BranchID>::const_iterator it = iInfo.parentage().parents().begin(),
          itEnd = iInfo.parentage().parents().end();
          it != itEnd;
          ++it) {
         //Don't look for parents if we've previously looked at the parents
         if(oMap.find(*it) == oMap.end()) {
            //use side effect of calling operator[] which is if the item isn't there it will add it as 'false'
            oMap[*it];
            boost::shared_ptr<ProductProvenance> pInfo = iMapper.branchToEntryInfo(*it);
            if(pInfo.get()) {
               markAncestors(*pInfo,iMapper,oMap,oMapperMissing);
            } else {
               oMapperMissing.insert(*it);
            }
         }
      }
   }

    void
   ProvenanceCheckerOutputModule::write(EventPrincipal const& e) {
      //check ProductProvenance's parents to see if they are in the ProductProvenance list
      boost::shared_ptr<BranchMapper> mapperPtr= e.branchMapperPtr();

      std::map<BranchID,bool> seenParentInPrincipal;
      std::set<BranchID> missingFromMapper;
      std::set<BranchID> missingProductProvenance;

      for(EventPrincipal::const_iterator it = e.begin(), itEnd = e.end();
          it != itEnd;
          ++it) {
         if(it->second && !it->second->productUnavailable()) {
            //This call seems to have a side effect of filling the 'ProductProvenance' in the Group
            OutputHandle const oh = e.getForOutput(it->first, false);

            if(not it->second->productProvenancePtr().get() ) {
               missingProductProvenance.insert(it->first);
               continue;
            }
            boost::shared_ptr<ProductProvenance> pInfo = mapperPtr->branchToEntryInfo(it->first);
            if(!pInfo.get()) {
               missingFromMapper.insert(it->first);
            }
            markAncestors(*(it->second->productProvenancePtr()),*mapperPtr,seenParentInPrincipal, missingFromMapper);
         }
         seenParentInPrincipal[it->first]=true;
      }

      //Determine what BranchIDs are in the product registry
      const ProductRegistry& reg = e.productRegistry();
      const ProductRegistry::ProductList prodList = reg.productList();
      std::set<BranchID> branchesInReg;
      for(ProductRegistry::ProductList::const_iterator it = prodList.begin(), itEnd = prodList.end();
          it != itEnd;
          ++it) {
         branchesInReg.insert(it->second.branchID());
      }

      std::set<BranchID> missingFromPrincipal;
      std::set<BranchID> missingFromReg;
      for(std::map<BranchID,bool>::iterator it=seenParentInPrincipal.begin(), itEnd = seenParentInPrincipal.end();
          it != itEnd;
          ++it) {
         if(!it->second) {
            missingFromPrincipal.insert(it->first);
         }
         if(branchesInReg.find(it->first) == branchesInReg.end()) {
            missingFromReg.insert(it->first);
         }
      }

      if(missingFromMapper.size()) {
         mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from BranchMapper\n";
         for(std::set<BranchID>::iterator it=missingFromMapper.begin(), itEnd = missingFromMapper.end();
             it!=itEnd;
             ++it) {
            mf::LogProblem("ProvenanceChecker") << *it;
         }
      }
      if(missingFromPrincipal.size()) {
         mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from EventPrincipal\n";
         for(std::set<BranchID>::iterator it=missingFromPrincipal.begin(), itEnd = missingFromPrincipal.end();
             it!=itEnd;
             ++it) {
            mf::LogProblem("ProvenanceChecker") << *it;
         }
      }

      if(missingProductProvenance.size()) {
         mf::LogError("ProvenanceChecker") << "The Groups for the following BranchIDs have no ProductProvenance\n";
         for(std::set<BranchID>::iterator it=missingProductProvenance.begin(), itEnd = missingProductProvenance.end();
             it!=itEnd;
             ++it) {
            mf::LogProblem("ProvenanceChecker") << *it;
         }
      }

      if(missingFromReg.size()) {
         mf::LogError("ProvenanceChecker") << "Missing the following BranchIDs from ProductRegistry\n";
         for(std::set<BranchID>::iterator it=missingFromReg.begin(), itEnd = missingFromReg.end();
             it!=itEnd;
             ++it) {
            mf::LogProblem("ProvenanceChecker") << *it;
         }
      }

      if(missingFromMapper.size() or missingFromPrincipal.size() or missingProductProvenance.size() or missingFromReg.size()) {
         throw cet::exception("ProvenanceError")
           << (missingFromMapper.size() or missingFromPrincipal.size()?"Having missing ancestors": "")
           << (missingFromMapper.size()?" from BranchMapper":"")
           << (missingFromMapper.size() and missingFromPrincipal.size()?" and":"")
           << (missingFromPrincipal.size()?" from EventPrincipal":"")
           << (missingFromMapper.size() or missingFromPrincipal.size()?".\n":"")
           << (missingProductProvenance.size()?" Have missing ProductProvenance's from Group in EventPrincipal.\n":"")
           << (missingFromReg.size()?" Have missing info from ProductRegistry.\n":"");
      }
   }

}  // namespace art

// ======================================================================

using art::ProvenanceCheckerOutputModule;
DEFINE_ART_MODULE(ProvenanceCheckerOutputModule);

// ======================================================================
