// Casio CZ style SawPulse oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZSawPulse = _CZOscPlain<_CZWaveOp::SawPulse>;
using CZSawPulseWidget = _CZOscPlainWidget<_CZWaveOp::SawPulse>;

Model* modelCZSawPulse = createModel<CZSawPulse, CZSawPulseWidget>("CZSawPulse");
