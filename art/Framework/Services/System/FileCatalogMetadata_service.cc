#include "art/Framework/Services/System/FileCatalogMetadata.h"

#include "art/Utilities/Exception.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

art::FileCatalogMetadata::FileCatalogMetadata(art::FileCatalogMetadata::Parameters const & config,
                                              ActivityRegistry &)
  : checkSyntax_{config().checkSyntax()}
  , md_{}
  , mdFromInput_{config().metadataFromInput()}
{
  std::string appFam;
  if ( config().applicationFamily(appFam) ) addMetadataString("applicationFamily" , appFam);

  std::string appVer;
  if ( config().applicationVersion(appVer) ) addMetadataString("applicationVersion", appVer);

  // Always write out fileType -- may be overridden.
  std::string fileType {"unknown"};
  config().fileType(fileType);
  addMetadataString("file_type", fileType);

  std::string rt;
  if ( config().runType(rt) ) {
    addMetadataString("run_type", rt);
  }

  std::string g;
  if ( config().group(g) ) addMetadataString("group", g);

  std::string pid;
  if ( config().processID(pid) ) addMetadataString("process_id", pid);

}

void
art::FileCatalogMetadata::
addMetadata(std::string const & key, std::string const & value)
{
  if (checkSyntax_) {
    rapidjson::Document d;
    std::string checkString("{ ");
    checkString += cet::canonical_string(key);
    checkString += " : ";
    checkString += value;
    checkString += " }";
    if (d.Parse(checkString.c_str()).HasParseError()) {
      auto const nSpaces = d.GetErrorOffset();
      throw Exception(errors::DataCorruption)
        << "art::FileCatalogMetadata::addMetadata() JSON syntax error:\n"
        << rapidjson::GetParseError_En(d.GetParseError())
        << " Faulty key/value clause:\n"
        << checkString << "\n"
        << (nSpaces ? std::string(nSpaces, '-') : "")
        << "^\n";
    }
  }
  md_.emplace_back(key, value);
}

// Standard constructor / maker is just fine.
DEFINE_ART_SERVICE(art::FileCatalogMetadata)
