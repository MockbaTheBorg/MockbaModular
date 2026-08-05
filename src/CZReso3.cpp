// Casio CZ style Reso3 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZReso3 = _CZOscReso<_CZWaveOp::Reso3>;
using CZReso3Widget = _CZOscResoWidget<_CZWaveOp::Reso3>;

Model* modelCZReso3 = createModel<CZReso3, CZReso3Widget>("CZReso3");
