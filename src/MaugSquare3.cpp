// Moog style Square .17 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugSquare3 = _MaugOscSingle<_MaugWaveOp::Square3>;
using MaugSquare3Widget = _MaugOscSingleWidget<_MaugWaveOp::Square3>;

Model* modelMaugSquare3 = createModel<MaugSquare3, MaugSquare3Widget>("MaugSquare3");
