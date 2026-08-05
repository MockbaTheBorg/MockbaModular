// Moog style Triangle oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugTriangle = _MaugOscSingle<_MaugWaveOp::Triangle>;
using MaugTriangleWidget = _MaugOscSingleWidget<_MaugWaveOp::Triangle>;

Model* modelMaugTriangle = createModel<MaugTriangle, MaugTriangleWidget>("MaugTriangle");
