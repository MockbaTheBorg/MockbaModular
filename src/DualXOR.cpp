// Simple XOR gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualXOR = _DualGate2<_GateOp2::XOR>;
using DualXORWidget = _DualGate2Widget<_GateOp2::XOR>;

Model* modelDualXOR = createModel<DualXOR, DualXORWidget>("DualXOR");
