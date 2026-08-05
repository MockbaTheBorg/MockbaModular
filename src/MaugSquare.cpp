// Moog style Square .48 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugSquare = _MaugOscSingle<_MaugWaveOp::Square>;
using MaugSquareWidget = _MaugOscSingleWidget<_MaugWaveOp::Square>;

Model* modelMaugSquare = createModel<MaugSquare, MaugSquareWidget>("MaugSquare");
