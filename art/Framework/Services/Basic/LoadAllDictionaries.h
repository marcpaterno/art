#ifndef FWCore_Services_LoadAllDictionaries_h
#define FWCore_Services_LoadAllDictionaries_h
// -*- C++ -*-
//
// Package:     Services
// Class  :     LoadAllDictionaries
//
/**\class LoadAllDictionaries LoadAllDictionaries.h FWCore/Services/interface/LoadAllDictionaries.h

 Description: Loads all Capability dictionaries

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu Sep 15 09:47:42 EDT 2005
//
//

// system include files

// user include files

// forward declarations
namespace edm {
   class ParameterSet;
   namespace service {
      class LoadAllDictionaries
   {

   public:
      LoadAllDictionaries(const fhicl::ParameterSet&);
      //virtual ~LoadAllDictionaries();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------

   private:
      LoadAllDictionaries(const LoadAllDictionaries&); // stop default

      const LoadAllDictionaries& operator=(const LoadAllDictionaries&); // stop default

      // ---------- member data --------------------------------

   };
   }
}

#endif
