//////////////////////////////////////////////////////////////////////
//
// InputFileCatalog
//
//////////////////////////////////////////////////////////////////////

#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/FileServiceInterfaces/FileDeliveryStatus.h"
#include "art/Framework/Services/FileServiceInterfaces/FileTransfer.h"
#include "art/Framework/Services/FileServiceInterfaces/FileTransferStatus.h"
#include "boost/algorithm/string.hpp"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"

#include <cassert>

namespace art {

  InputFileCatalog::InputFileCatalog(
    fhicl::TableFragment<InputFileCatalog::Config> const& config)
    : fileSources_{config().namesParameter()}
  {

    if (fileSources_.empty()) {
      throw Exception(errors::CatalogServiceError,
                      "InputFileCatalog::InputFileCatalog()\n")
        << "Empty '" << config().namesParameter.name()
        << "' parameter specified for input source.\n";
    }

    // Configure FileDelivery service
    ci_->configure(fileSources_);
    searchable_ = ci_->isSearchable();

    if (searchable_)
      fileCatalogItems_.resize(fileSources_.size());
  }

  FileCatalogItem const&
  InputFileCatalog::currentFile() const
  {
    if (fileIdx_ == indexEnd) {
      throw Exception(
        errors::LogicError,
        "Cannot access the current file while the file catalog is empty!");
    }
    assert(fileIdx_ <= maxIdx_);
    return fileCatalogItems_[fileIdx_];
  }

  size_t
  InputFileCatalog::currentIndex() const noexcept
  {
    return fileIdx_;
  }

  bool
  InputFileCatalog::getNextFile(int const attempts)
  {
    // get next file from FileDelivery service and give it to
    // currentFile_ object returns false if theres no more file
    //
    // If hasNextFile() has been called prior to getNextFile(), it
    // does not actually go fetch the next file from FileDelivery
    // service, instead, it advances the iterator by one and make the
    // "hidden" next file current.

    if (nextFileProbed_ && !hasNextFile_)
      return false;

    if ((nextFileProbed_ && hasNextFile_) ||
        retrieveNextFile(nextItem_, attempts)) {
      nextFileProbed_ = false;
      fileIdx_ =
        (fileIdx_ == indexEnd) ? 0 : (searchable_ ? (fileIdx_ + 1) : 0);
      if (fileIdx_ > maxIdx_)
        maxIdx_ = fileIdx_;
      fileCatalogItems_[fileIdx_] = nextItem_;
      return true;
    }

    return false;
  }

  bool
  InputFileCatalog::hasNextFile(int const attempts)
  {
    // A probe. It tries (and actually does) retrieve the next file
    // from the FileDelivery service. But does not advance the current
    // file pointer
    if (nextFileProbed_)
      return hasNextFile_;

    hasNextFile_ = retrieveNextFile(nextItem_, attempts);
    nextFileProbed_ = true;

    return hasNextFile_;
  }

  bool
  InputFileCatalog::retrieveNextFile(FileCatalogItem& item,
                                     int const attempts,
                                     bool const transferOnly)
  {
    // Tell the service the current opened file (if there is one) is consumed
    finish();

    // retrieve (deliver and transfer) next file from service
    // or, do the transfer only
    FileCatalogStatus status;
    if (transferOnly) {
      status = transferNextFile(item);
    } else {
      status = retrieveNextFileFromCacheOrService(item);
    }

    switch (status) {
    case FileCatalogStatus::SUCCESS: {
      // mark the file as transferred
      ci_->updateStatus(item.uri(), FileDisposition::TRANSFERRED);
      return true;
    }
    case FileCatalogStatus::NO_MORE_FILES: {
      return false;
    }
    case FileCatalogStatus::DELIVERY_ERROR: {
      if (attempts <= 1) {
        throw Exception(errors::Configuration,
                        "InputFileCatalog::retrieveNextFile()\n")
          << "Delivery error encountered after reaching maximum number of "
             "attemtps!";
      }
      return retrieveNextFile(item, attempts - 1, false);
    }
    case FileCatalogStatus::TRANSFER_ERROR: {
      if (attempts <= 1) {
        // if we end up with a transfer error, the method returns
        // with a true flag and empty filename. Weired enough, but
        // the next file does exist we just cannot retrieve it. Therefore
        // we notify the service that the file has been skipped
        ci_->updateStatus(item.uri(), FileDisposition::SKIPPED);
        return true;
      }
      return retrieveNextFile(item, attempts - 1, true);
    }
    }
    assert(false);
    return false; // Unreachable, but keeps the compiler happy
  }

  FileCatalogStatus
  InputFileCatalog::retrieveNextFileFromCacheOrService(FileCatalogItem& item)
  {
    // Try to get it from cached files
    if (fileIdx_ < maxIdx_) {
      item = fileCatalogItems_[fileIdx_ + 1];
      return FileCatalogStatus::SUCCESS;
    }

    // Try to get it from the service
    std::string uri;
    double wait = 0.0;

    // get file delivered
    int const result = ci_->getNextFileURI(uri, wait);

    if (result == FileDeliveryStatus::NO_MORE_FILES)
      return FileCatalogStatus::NO_MORE_FILES;

    if (result != FileDeliveryStatus::SUCCESS)
      return FileCatalogStatus::DELIVERY_ERROR;

    item = FileCatalogItem("", "", uri);

    // get file transfered
    return transferNextFile(item);
  }

  FileCatalogStatus
  InputFileCatalog::transferNextFile(FileCatalogItem& item)
  {
    std::string pfn;
    int const result = ft_->translateToLocalFilename(item.uri(), pfn);

    if (result != FileTransferStatus::SUCCESS) {
      item.fileName("");
      item.logicalFileName("");
      item.skip();
      return FileCatalogStatus::TRANSFER_ERROR;
    }

    // successfully retrieved the file
    std::string lfn = pfn;
    boost::trim(pfn);

    if (pfn.empty()) {
      throw Exception(errors::Configuration,
                      "InputFileCatalog::retrieveNextFileFromCacheService()\n")
        << "An empty string specified in parameter for input source.\n";
    }

    if (isPhysical(pfn)) {
      lfn.clear();
    }

    item.fileName(pfn);
    item.logicalFileName(lfn);
    return FileCatalogStatus::SUCCESS;
  }

  void
  InputFileCatalog::rewind()
  {
    if (!searchable_) {
      throw Exception(errors::LogicError, "InputFileCatalog::rewind()\n")
        << "A non-searchable catalog is not allowed to rewind!";
    }
    fileIdx_ = 0;
  }

  void
  InputFileCatalog::rewindTo(size_t const index)
  {
    // rewind to a previous file location in the catalog service is
    // not rewinded. only usable when
    // FileDeliveryService::areFilesPersistent() is true
    if (!searchable_) {
      throw Exception(errors::LogicError, "InputFileCatalog::rewindTo()\n")
        << "A non-searchable catalog is not allowed to rewind!";
    }

    if (index > maxIdx_) {
      throw Exception(errors::InvalidNumber, "InputFileCatalog::rewindTo()\n")
        << "Index " << index << " is out of range!";
    }

    fileIdx_ = index;
  }

  void
  InputFileCatalog::finish()
  {
    if (fileIdx_ != indexEnd          // there is a current file
        && !currentFile().skipped()   // not skipped
        && !currentFile().consumed()) // not consumed
    {
      ci_->updateStatus(currentFile().uri(), FileDisposition::CONSUMED);
      fileCatalogItems_[fileIdx_].consume();
    }
  }

} // namespace art
