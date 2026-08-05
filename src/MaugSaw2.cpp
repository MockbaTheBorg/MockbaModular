// Moog style InvSaw oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using MaugSaw2 = _MaugOscSingle<_MaugWaveOp::InvSaw>;
using MaugSaw2Widget = _MaugOscSingleWidget<_MaugWaveOp::InvSaw>;

Model* modelMaugSaw2 = createModel<MaugSaw2, MaugSaw2Widget>("MaugSaw2");
