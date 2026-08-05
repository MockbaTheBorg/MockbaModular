// Moog style Shark oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugShark = _MaugOscSingle<_MaugWaveOp::Shark>;
using MaugSharkWidget = _MaugOscSingleWidget<_MaugWaveOp::Shark>;

Model* modelMaugShark = createModel<MaugShark, MaugSharkWidget>("MaugShark");
