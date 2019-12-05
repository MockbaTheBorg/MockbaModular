// VCV Rack plugins by Mockba the Borg

#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelBlank);
	p->addModel(modelFeidah);
	p->addModel(modelFeidahS);
	p->addModel(modelFiltah);
	p->addModel(modelMixah);
	p->addModel(modelDividah);
	p->addModel(modelCountah);
	p->addModel(modelSelectah);
	p->addModel(modelShapah);
	p->addModel(modelCZSaw);
	p->addModel(modelCZSquare);
	p->addModel(modelCZPulse);
	p->addModel(modelCZDblSine);
	p->addModel(modelCZSawPulse);
	p->addModel(modelCZReso1);
	p->addModel(modelCZReso2);
	p->addModel(modelCZReso3);
	p->addModel(modelComparator);
	p->addModel(modelDualBUFFER);
	p->addModel(modelDualNOT);
	p->addModel(modelDualOR);
	p->addModel(modelDualNOR);
	p->addModel(modelDualAND);
	p->addModel(modelDualNAND);
	p->addModel(modelDualXOR);
	p->addModel(modelDualXNOR);
	p->addModel(modelUDPClockSlave);
	p->addModel(modelUDPClockMaster);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
