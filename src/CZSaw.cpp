// Casio CZ style Saw oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZSaw = _CZOscLFO<_CZWaveOp::Saw>;
using CZSawWidget = _CZOscLFOWidget<_CZWaveOp::Saw>;

Model* modelCZSaw = createModel<CZSaw, CZSawWidget>("CZSaw");
