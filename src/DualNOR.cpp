// Simple NOR gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualNOR = _DualGate2<_GateOp2::NOR>;
using DualNORWidget = _DualGate2Widget<_GateOp2::NOR>;

Model* modelDualNOR = createModel<DualNOR, DualNORWidget>("DualNOR");
