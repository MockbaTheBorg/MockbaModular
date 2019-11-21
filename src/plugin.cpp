// VCV Rack plugins by Mockba the Borg

#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelBlank);
	p->addModel(modelFeidah);
	p->addModel(modelMixah);
	p->addModel(modelDividah);
	p->addModel(modelCountah);
	p->addModel(modelSelectah);
	p->addModel(modelCZSaw);
	p->addModel(modelCZSquare);
	p->addModel(modelCZPulse);
	p->addModel(modelCZReso);
	p->addModel(modelUDPClockSlave);
	p->addModel(modelUDPClockMaster);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
