// Simple XNOR gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualXNOR = _DualGate2<_GateOp2::XNOR>;
using DualXNORWidget = _DualGate2Widget<_GateOp2::XNOR>;

Model* modelDualXNOR = createModel<DualXNOR, DualXNORWidget>("DualXNOR");
