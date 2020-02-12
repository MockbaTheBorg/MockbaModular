// A blank, why not? by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct Blank : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Blank() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {

	}
};

struct BlankWidget : ModuleWidget {
	int bgSel;

	BlankWidget(Blank* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Blank.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}

	void loadBg() {
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, loadBack(bgSel))));
	}

	void appendContextMenu(Menu* menu) override;
};

struct BackMenuValueItem : MenuItem {
	BlankWidget* moduleWidget;
	int bgSel;
	void onAction(const event::Action& e) override {
		moduleWidget->bgSel = bgSel;
		moduleWidget->loadBg();
	}
};

struct BackMenuItem : MenuItem {
	BlankWidget* moduleWidget;
	Menu* createChildMenu() override {
		Menu* menu = new Menu;
		for (int bgSel = 0; bgSel < 3; ++bgSel) {
			BackMenuValueItem* item = new BackMenuValueItem;
			switch (bgSel) {
			case 0:
				item->text = "Light";
				break;
			case 1:
				item->text = "Dark";
				break;
			default:
				item->text = "Aged";
			}
			item->rightText = CHECKMARK(moduleWidget->bgSel == bgSel);
			item->moduleWidget = moduleWidget;
			item->bgSel = bgSel;
			menu->addChild(item);
		}
		return menu;
	}
};

void BlankWidget::appendContextMenu(Menu* menu) {
	MenuLabel* spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	BackMenuItem* menuItem = new BackMenuItem;
	menuItem->text = "Background theme";
	menuItem->rightText = RIGHT_ARROW;
	menuItem->moduleWidget = this;
	menu->addChild(menuItem);
}

Model* modelBlank = createModel<Blank, BlankWidget>("Blank");