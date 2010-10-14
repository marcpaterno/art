#include "art/Framework/Core/WorkerMaker.h"

using fhicl::ParameterSet;


namespace edm {

Maker::~Maker()
{ }

ModuleDescription
Maker::createModuleDescription(WorkerParams const &p) const
{
  ParameterSet const& procParams = *p.procPset_;
  ParameterSet const& conf = *p.pset_;
  ModuleDescription md;
  md.parameterSetID_ = conf.id();
  md.moduleName_ = conf.get<std::string>("@module_type");
  md.moduleLabel_ = conf.get<std::string>("@module_label");
  md.processConfiguration_ = ProcessConfiguration(p.processName_, procParams.id(), p.releaseVersion_, p.passID_);
  return md;
}

void
Maker::throwConfigurationException(ModuleDescription const &md,
                                   sigc::signal<void, ModuleDescription const&>& post,
                                   cms::Exception const& iException) const
{
  edm::Exception toThrow(edm::errors::Configuration,"Error occurred while creating ");
  toThrow<<md.moduleName_<<" with label "<<md.moduleLabel_<<"\n";
  toThrow.append(iException);
  post(md);
  throw toThrow;
}

} // namespace edm
