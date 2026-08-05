// Simple binary clock counter by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using Countah = _ClockDivCounter<true>;

struct CountahWidget : _ClockDivCounterWidget<true> {
	CountahWidget(Countah* module) : _ClockDivCounterWidget<true>(module, "res/Countah.svg") {}
};

Model* modelCountah = createModel<Countah, CountahWidget>("Countah");
