// Moog style Saw oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugSaw = _MaugOscSingle<_MaugWaveOp::Saw>;
using MaugSawWidget = _MaugOscSingleWidget<_MaugWaveOp::Saw>;

Model* modelMaugSaw = createModel<MaugSaw, MaugSawWidget>("MaugSaw");
