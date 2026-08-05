// Casio CZ style Pulse oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZPulse = _CZOscLFO<_CZWaveOp::Pulse>;
using CZPulseWidget = _CZOscLFOWidget<_CZWaveOp::Pulse>;

Model* modelCZPulse = createModel<CZPulse, CZPulseWidget>("CZPulse");
