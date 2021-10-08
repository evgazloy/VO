#include "plugin.hpp"
#include <iostream>

struct OneKnob : Module
{
    enum ParamIds
    {
		KNOB_PARAM,
		MODE_PARAM,
		NUM_PARAMS
    };

    enum InputIds
    {
		CV_INPUT,
		NUM_INPUTS
    };

    enum OutputIds
    {
		NUM_OUTPUTS
    };

    enum LightIds
    {
		NUM_LIGHTS
    };

    const float c_threshold = 1.f / 128.f;
    float m_knob {};
    float m_value {};
    ParamWidget *m_touchedParam {};
    bool m_picked {};
    bool m_isCV {};

    OneKnob()
    {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KNOB_PARAM, 0.f, 1.f, 0.f, "Map");
		configParam(MODE_PARAM, 0.f, 1.f, 0.f, "Absolute mode");
    }

    bool isPicked(float value)
    {
        if(!m_picked)
            m_picked = (std::abs(m_knob - value) < c_threshold);
        else
            m_picked = (m_value == value);

        return m_picked;
    };

    void process(const ProcessArgs& args) override
    {
        bool isCV = inputs[CV_INPUT].isConnected();
        if(isCV != m_isCV)
        {
            m_isCV = isCV;
            m_picked = false;
        }

        float knob = isCV ? inputs[CV_INPUT].getVoltage() / 10.f :
                                                      params[KNOB_PARAM].getValue();
        if(knob != m_knob)
        {
            m_knob = knob;

            ParamWidget *touchedParam = APP->scene->rack->touchedParam;
            if(touchedParam)
            {
                ParamQuantity *quantity = touchedParam->paramQuantity;
                if(quantity && quantity->isBounded())
                {
                    bool absMode = params[MODE_PARAM].getValue() > 0.f;
                    if(!absMode && (touchedParam != m_touchedParam))
                    {
                        m_touchedParam = touchedParam;
                        m_picked = false;
                    }

                    if(quantity->module != this)
                    {
                        if(absMode || this->isPicked(quantity->getScaledValue()))
                        {
                            quantity->setScaledValue(knob);
                            m_value = quantity->getScaledValue();
                        }
                    }
                }
            }
        }
	}
};


struct OneKnobWidget : ModuleWidget
{
    OneKnobWidget(OneKnob* module)
    {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/OneKnob.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 33.853)), module, OneKnob::KNOB_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(7.62, 98.562)), module, OneKnob::MODE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 48.741)), module, OneKnob::CV_INPUT));
	}
};


Model* modelOneKnob = createModel<OneKnob, OneKnobWidget>("VO-OneKnob");
