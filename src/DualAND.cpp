// Simple AND gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualAND = _DualGate2<_GateOp2::AND>;
using DualANDWidget = _DualGate2Widget<_GateOp2::AND>;

Model* modelDualAND = createModel<DualAND, DualANDWidget>("DualAND");
