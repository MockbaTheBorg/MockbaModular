// Casio CZ style DblSine oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZDblSine = _CZOscPlain<_CZWaveOp::DblSine>;
using CZDblSineWidget = _CZOscPlainWidget<_CZWaveOp::DblSine>;

Model* modelCZDblSine = createModel<CZDblSine, CZDblSineWidget>("CZDblSine");
