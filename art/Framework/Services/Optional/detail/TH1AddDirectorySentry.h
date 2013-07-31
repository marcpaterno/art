#ifndef art_Framework_Services_Optional_detail_TH1AddDirectorySentry_h
#define art_Framework_Services_Optional_detail_TH1AddDirectorySentry_h

// -*- C++ -*-
//
// Package:     UtilAlgos
// Class  :     TH1AddDirectorySentry
//
/*
 Description: Manages the status of the ROOT directory

 Usage: Construct an instance of this object in a routine in which you
    expect a ROOT histogram to be automatically added to the current
    directory in a file. The destructor will be sure to reset ROOT to
    its previous setting.

*/

//
// Original Author:  Chris Jones
//         Created:  Thu Nov  8 12:16:13 EST 2007
//
//


class TH1AddDirectorySentry
{

public:
  TH1AddDirectorySentry();
  ~TH1AddDirectorySentry();


private:
  TH1AddDirectorySentry(const TH1AddDirectorySentry&) = delete;
  TH1AddDirectorySentry& operator=(const TH1AddDirectorySentry&) = delete;
  bool status_;
};


#endif /* art_Framework_Services_Optional_detail_TH1AddDirectorySentry_h */

/// emacs configuration

/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
