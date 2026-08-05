// Simple OR gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualOR = _DualGate2<_GateOp2::OR>;
using DualORWidget = _DualGate2Widget<_GateOp2::OR>;

Model* modelDualOR = createModel<DualOR, DualORWidget>("DualOR");
