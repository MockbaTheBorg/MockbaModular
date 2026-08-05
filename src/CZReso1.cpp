// Casio CZ style Reso1 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZReso1 = _CZOscReso<_CZWaveOp::Reso1>;
using CZReso1Widget = _CZOscResoWidget<_CZWaveOp::Reso1>;

Model* modelCZReso1 = createModel<CZReso1, CZReso1Widget>("CZReso1");
