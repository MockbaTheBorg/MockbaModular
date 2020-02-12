// MockbaModular plugins for VCV Rack by Mockba the Borg

#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelBlank);
	p->addModel(modelFeidah);
	p->addModel(modelFeidahS);
	p->addModel(modelFiltah);
	p->addModel(modelMixah);
	p->addModel(modelMixah3);
	p->addModel(modelDividah);
	p->addModel(modelCountah);
	p->addModel(modelSelectah);
	p->addModel(modelShapah);
	p->addModel(modelHoldah);
	p->addModel(modelPannah);
	p->addModel(modelCZSaw);
	p->addModel(modelCZSquare);
	p->addModel(modelCZPulse);
	p->addModel(modelCZDblSine);
	p->addModel(modelCZSawPulse);
	p->addModel(modelCZReso1);
	p->addModel(modelCZReso2);
	p->addModel(modelCZReso3);
	p->addModel(modelCZOsc);
	p->addModel(modelMaugTriangle);
	p->addModel(modelMaugShark);
	p->addModel(modelMaugSaw);
	p->addModel(modelMaugSaw2);
	p->addModel(modelMaugSquare);
	p->addModel(modelMaugSquare2);
	p->addModel(modelMaugSquare3);
	p->addModel(modelMaugOsc);
	p->addModel(modelComparator);
	p->addModel(modelDualBUFFER);
	p->addModel(modelDualNOT);
	p->addModel(modelDualOR);
	p->addModel(modelDualNOR);
	p->addModel(modelDualAND);
	p->addModel(modelDualNAND);
	p->addModel(modelDualXOR);
	p->addModel(modelDualXNOR);
	p->addModel(modelPSelectah);
	p->addModel(modelUDPClockSlave);
	p->addModel(modelUDPClockMaster);
}
