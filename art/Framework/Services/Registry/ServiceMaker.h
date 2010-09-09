#ifndef ServiceRegistry_ServiceMaker_h
#define ServiceRegistry_ServiceMaker_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     ServiceMaker
//
/**\class ServiceMaker ServiceMaker.h FWCore/ServiceRegistry/interface/ServiceMaker.h

 Description: Used to make an instance of a Service

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Mon Sep  5 13:33:00 EDT 2005
//
//

// system include files
#include <memory>

// user include files
#include "art/Framework/Services/Registry/ServiceMakerBase.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/ServicePluginFactory.h"

// forward declarations

namespace edm {
   class ParameterSet;
   class ActivityRegistry;


   namespace serviceregistry {
      template<class T, class TConcrete >
      struct MakerBase {
         typedef T interface_t;
         typedef TConcrete concrete_t;
      };

      template< class T, class TConcrete = T>
         struct AllArgsMaker : public MakerBase<T,TConcrete> {

         std::auto_ptr<T> make(const fhicl::ParameterSet& iPS,
                               edm::ActivityRegistry& iAR) const
         {
            return std::auto_ptr<T>(new TConcrete(iPS, iAR));
         }
      };

      template< class T, class TConcrete = T>
      struct ParameterSetMaker : public MakerBase<T,TConcrete> {
         std::auto_ptr<T> make(const fhicl::ParameterSet& iPS,
                               edm::ActivityRegistry& /* iAR */) const
         {
            return std::auto_ptr<T>(new TConcrete(iPS));
         }
      };

      template< class T, class TConcrete = T>
      struct NoArgsMaker : public MakerBase<T,TConcrete> {
         std::auto_ptr<T> make(const fhicl::ParameterSet& /* iPS */,
                               edm::ActivityRegistry& /* iAR */) const
         {
            return std::auto_ptr<T>(new TConcrete());
         }
      };


      template< class T, class TMaker = AllArgsMaker<T> >
      class ServiceMaker : public ServiceMakerBase
      {

public:
         ServiceMaker() {}
         //virtual ~ServiceMaker();

         // ---------- const member functions ---------------------
         virtual const std::type_info& serviceType() const { return typeid(T); }

         virtual bool make(const fhicl::ParameterSet& iPS,
                           edm::ActivityRegistry& iAR,
                           ServicesManager& oSM) const
         {
            TMaker maker;
            std::auto_ptr<T> pService(maker.make(iPS, iAR));
            boost::shared_ptr<ServiceWrapper<T> > ptr(new ServiceWrapper<T>(pService));
            return oSM.put(ptr);
         }

         // ---------- static member functions --------------------

         // ---------- member functions ---------------------------

private:
         ServiceMaker(const ServiceMaker&); // stop default

         const ServiceMaker& operator=(const ServiceMaker&); // stop default

         // ---------- member data --------------------------------

      };
   }
}


#define DEFINE_FWK_SERVICE(type) \
DEFINE_EDM_PLUGIN (edm::serviceregistry::ServicePluginFactory,edm::serviceregistry::ServiceMaker<type>,#type)

#define DEFINE_ANOTHER_FWK_SERVICE(type) \
DEFINE_EDM_PLUGIN (edm::serviceregistry::ServicePluginFactory,edm::serviceregistry::ServiceMaker<type>,#type)

#define DEFINE_FWK_SERVICE_MAKER(concrete,maker) \
typedef edm::serviceregistry::ServiceMaker<maker::interface_t,maker> concrete ## _ ## _t; \
DEFINE_EDM_PLUGIN (edm::serviceregistry::ServicePluginFactory, concrete ## _ ##  _t ,#concrete)

#define DEFINE_ANOTHER_FWK_SERVICE_MAKER(concrete,maker) \
typedef edm::serviceregistry::ServiceMaker<maker::interface_t,maker> concrete ## _ ##  _t; \
DEFINE_EDM_PLUGIN (edm::serviceregistry::ServicePluginFactory, concrete ## _ ## _t ,#concrete)

#endif
