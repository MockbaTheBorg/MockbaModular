// Casio CZ style Square oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using CZSquare = _CZOscLFO<_CZWaveOp::Square>;
using CZSquareWidget = _CZOscLFOWidget<_CZWaveOp::Square>;

Model* modelCZSquare = createModel<CZSquare, CZSquareWidget>("CZSquare");
