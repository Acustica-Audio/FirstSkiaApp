#include "include/Layers/LinesLayer.h"
#include "include/Utils/Shaders.h"
#include "include/Utils/Utils.h"
#include "include/Utils/ThemeUtils.h"
#include "include/Controls/Slider.h"
#include "include/Controls/Button.h"
#include "microprofile.h"

namespace
{
	enum class Controls
	{
		kX,
		kY,
		kRadius,
		kTwist
	};

	SliderParams CreateSliderParams(Controls control)
	{
		SliderParams params;
		switch (control)
		{
		case Controls::kX:
			params.m_Value = Shaders::kSwirlDefault.x;
			break;
		case Controls::kY:
			params.m_Value = Shaders::kSwirlDefault.y;
			break;
		case Controls::kRadius:
			params.m_Value = Shaders::kSwirlDefault.radius;
			break;
		case Controls::kTwist:
			params.m_Min = -2.0;
			params.m_Max = 2.0;
			params.m_Value = Shaders::kSwirlDefault.twist;
			break;
		}
		return params;
	}
}

LinesLayer::LinesLayer() : m_Container(ThemeUtils::GetRightContainerParams())
{
	//m_Image = Utils::LoadImageFromFile(SkString{"resources/4k.jpg"});

	auto&& effectContainer = m_Container.AddControl<ControlsContainer>(ThemeUtils::GetControlsContainerParams());
	m_XSlider = effectContainer.lock()->AddControl<Slider>(CreateSliderParams(Controls::kX), SkString{"X:"});
	m_YSlider = effectContainer.lock()->AddControl<Slider>(CreateSliderParams(Controls::kY), SkString{"Y:"});
	m_RadiusSlider = effectContainer.lock()->AddControl<Slider>(CreateSliderParams(Controls::kRadius), SkString{"Radius:"});
	m_TwistsSlider = effectContainer.lock()->AddControl<Slider>(CreateSliderParams(Controls::kTwist), SkString{"Twists:"});
	effectContainer.lock()->AddControl<Button>([this]() {
		m_XSlider.lock()->SetValue(Shaders::kSwirlDefault.x);
		m_YSlider.lock()->SetValue(Shaders::kSwirlDefault.y);
		m_RadiusSlider.lock()->SetValue(Shaders::kSwirlDefault.radius);
		m_TwistsSlider.lock()->SetValue(Shaders::kSwirlDefault.twist);
	}, SkString{"Reset"});

	auto res = Shaders::LoadFromFile(SkString{"resources/shaders/Swirl.sksl"});
	m_Effect = std::move(res.effect);
}

void LinesLayer::createRandomCurves(SkRect bounds, int numCurves) {
	MICROPROFILE_ENTERI("skia","update", 0xFF000000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-1.0, 1.0); // Adjust the range as needed

	for (int i = 0; i < numCurves; ++i) {
        float height = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, bounds.height());
        float stroke = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, 15.0);
        uint8_t r = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, 255.0);
        uint8_t g = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, 255.0);
        uint8_t b = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, 255.0);
        uint8_t a = Utils::MapRange(dis(gen), -1.0, 1.0, 0.0, 255.0);
        CubicCurve c;
        c.x1 = 0;
        c.y1 = height;
        c.x2 = bounds.width() / 2,
        c.y2 = height;
        c.x3 = bounds.width();
        c.y3 = height;
		SkColor skiaColor = SkColorSetARGB(a, r, g, b);
    	c.paint.setColor(skiaColor);
    	c.paint.setStrokeWidth(stroke);
    	c.paint.setStyle(SkPaint::kStroke_Style);
		c.paint.setAntiAlias(true);
        //c.stroke = stroke;
        //c.color = (static_cast<uint32_t>(r) << 24) | (static_cast<uint32_t>(g) << 16) | (static_cast<uint32_t>(b) << 8) | static_cast<uint32_t>(a);
        c.dirY = dis(gen) * 3;
        m_curves.push_back(c);
    }
    MICROPROFILE_LEAVE();
}

void LinesLayer::updateCurves(SkRect bounds) {
    for (CubicCurve &c : m_curves) {
        c.y2 += c.dirY;
        if (c.y2 < 0 || c.y2 > bounds.height()) {
            c.dirY = -c.dirY;
        }
    }
}

void LinesLayer::onPaint(SkSurface* surface)
{
	MICROPROFILE_ENTERI("skia", "draw", 0xFF000000);
	SkCanvas* canvas = surface->getCanvas();

	// clear canvas with black color
	canvas->clear(SkColors::kBlack);
	const SkRect bounds = Utils::GetBounds(canvas);

	if (!m_curves_created) {
		createRandomCurves(bounds, 10);
		m_curves_created = true;
	}

    for (auto c : m_curves) {
    	// Extract individual color components (RGBA)
		SkPath path;
    	path.moveTo(c.x1, c.y1);  // Starting point
    	path.cubicTo(c.x1, c.y1, c.x2, c.y2, c.x3, c.y3);  // Cubic Bezier control points and end point
    	// Draw the Bezier curve on the canvas
    	canvas->drawPath(path, c.paint);
    }
    MICROPROFILE_LEAVE();

	/*
	if (m_Effect)
	{
		SkAutoCanvasRestore guard(canvas, true);
		const SkRect imageRect = SkRect::MakeWH(m_Image->width(), m_Image->height());
		canvas->setMatrix(SkMatrix::RectToRect(imageRect, bounds, SkMatrix::kCenter_ScaleToFit));

		Shaders::SwirlParameters params;
		params.width = imageRect.width();
		params.height = imageRect.height();
		params.x = m_XSlider.lock()->GetValue();
		params.y = m_YSlider.lock()->GetValue();
		params.radius = m_RadiusSlider.lock()->GetValue();
		params.twist = m_TwistsSlider.lock()->GetValue();

		SkPaint paint;
		paint.setShader(Shaders::CreateShader(m_Image, m_Effect, params));
		canvas->drawPaint(paint);
	}
	*/

	updateCurves(bounds);

	MicroProfileFlip(nullptr);

	// draw controls
	//m_Container.Draw(canvas, bounds);
}

bool LinesLayer::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifier)
{
	return m_Container.ProcessKey(key, state, modifier);
}

bool LinesLayer::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifier)
{
	return m_Container.ProcessMouse(x, y, state, modifier);
}
