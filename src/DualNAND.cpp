// Simple NAND gates by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

using DualNAND = _DualGate2<_GateOp2::NAND>;
using DualNANDWidget = _DualGate2Widget<_GateOp2::NAND>;

Model* modelDualNAND = createModel<DualNAND, DualNANDWidget>("DualNAND");
