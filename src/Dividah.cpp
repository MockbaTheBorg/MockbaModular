// Simple binary clock divider by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using Dividah = _ClockDivCounter<false>;

struct DividahWidget : _ClockDivCounterWidget<false> {
	DividahWidget(Dividah* module) : _ClockDivCounterWidget<false>(module, "res/Dividah.svg") {}
};

Model* modelDividah = createModel<Dividah, DividahWidget>("Dividah");
