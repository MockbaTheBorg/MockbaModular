// Moog style Square .29 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugSquare2 = _MaugOscSingle<_MaugWaveOp::Square2>;
using MaugSquare2Widget = _MaugOscSingleWidget<_MaugWaveOp::Square2>;

Model* modelMaugSquare2 = createModel<MaugSquare2, MaugSquare2Widget>("MaugSquare2");
