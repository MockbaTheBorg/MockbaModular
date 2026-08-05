// Casio CZ style Reso2 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZReso2 = _CZOscReso<_CZWaveOp::Reso2>;
using CZReso2Widget = _CZOscResoWidget<_CZWaveOp::Reso2>;

Model* modelCZReso2 = createModel<CZReso2, CZReso2Widget>("CZReso2");
