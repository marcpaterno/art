use strict;

use vars qw(@optional @system);

BEGIN { @optional = qw(RandomNumberGenerator TFileService);
	@system = qw(ConstProductRegistry CurrentModule FloatingPointControl TriggerNamesService);
    }

foreach my $service (@optional) {
  s&^(\s*#include\s+["<]art/Framework/Services/)Basic(/\Q${service}.h\E[">].*)$&${1}Optional${2}& and last;
}

foreach my $service (@system) {
  s&^(\s*#include\s+["<]art/Framework/Services/)Basic(/\Q${service}.h\E[">].*)$&${1}System${2}& and last;
}

### Local Variables:
### mode: cperl
### End:
