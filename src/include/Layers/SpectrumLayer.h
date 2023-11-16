#pragma once

#include "BaseLayer.h"
#include "include/Interfaces/IControlValue.h"
#include "include/Controls/ControlsContainer.h"
#include "include/effects/SkRuntimeEffect.h"
#include <vector>
#include <random>

class SpectrumLayer final : public BaseLayer
{
public:
	SpectrumLayer();
	~SpectrumLayer() override = default;

	SkString GetTitle() const override { return SkString{"Spectrum"}; };
	void onPaint(SkSurface* surface) override;
	bool onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifier) override;
	bool onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifier) override;

	void updateCurves(SkRect bounds);	
	void createRandomCurves(SkRect bounds, int numCurves);

private:
	struct CubicCurve {
	    double x1;
	    double y1;
	    double x2;
	    double y2;
	    double x3;
	    double y3;
	    float stroke;
	    SkPaint paint;
	    float dirY;
	};

	std::vector<CubicCurve> m_curves;
	bool m_curves_created = false;

	sk_sp<SkImage> m_Image;
	sk_sp<SkRuntimeEffect> m_Effect;

	ControlsContainer m_Container;
	std::weak_ptr<IControlValue> m_XSlider;
	std::weak_ptr<IControlValue> m_YSlider;
	std::weak_ptr<IControlValue> m_RadiusSlider;
	std::weak_ptr<IControlValue> m_TwistsSlider;
};
