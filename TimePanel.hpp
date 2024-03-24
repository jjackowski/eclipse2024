/*
 * This file is part of the Eclipse2024 project. It is subject to the GPLv3
 * license terms in the LICENSE file found in the top-level directory of this
 * distribution and at
 * https://github.com/jjackowski/eclipse2024/blob/master/LICENSE.
 * No part of the Eclipse2024 project, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 *
 * Copyright (C) 2024  Jeff Jackowski
 */
#include <duds/hardware/devices/clocks/Clock.hpp>
#include <duds/ui/graphics/PriorityGridLayout.hpp>

// not thread-safe
class TimePanel : public duds::ui::graphics::Panel {
	/**
	 * A set of four places to put the time.
	 * The first 3 are for center of display and placed to allow for something
	 * on the left or right. The first 2 include seconds. The last one is placed
	 * on the upper right.
	 */
	duds::ui::graphics::GridLayoutConfig glc = {
		{ // sizes
			// 24-hour formats
			{ // size step 0; timefont_12x17; includes seconds
				{ // minimum dimensions
					88, 17
				},
				{ // grid location
					1, 1
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelCenter |
				duds::ui::graphics::GridLayoutConfig::PanelExpand
			},
			/*
			{ // next size step; timefont_12x17; no seconds
				{ // minimum dimensions
					56, 17
				},
				{ // grid location
					1, 1
				}
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelCenter
			},
			*/
			{ // next size step 1; font_7x14; includes seconds
				{ // minimum dimensions
					56, 14
				},
				{ // grid location
					1, 1
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelCenter |
				duds::ui::graphics::GridLayoutConfig::PanelExpand
			},
			{ // next size step 2; font_7x14; no seconds
				{ // minimum dimensions
					35, 14
				},
				{ // grid location
					1, 1
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelCenter |
				duds::ui::graphics::GridLayoutConfig::PanelExpand
			},
			/*
			{ // next size step; font_Vx7; includes seconds
				{ // minimum dimensions
					28, 7
				},
				{ // grid location
					16, 0
				}
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelJustifyRight
			},
			*/
			{ // next size step 3; font_Vx8B; seconds
				{ // minimum dimensions
					59, 8
				},
				{ // grid location
					16, 0
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelJustifyRight
			},
			{ // next size step 4; font_Vx8; seconds
				{ // minimum dimensions
					39, 8
				},
				{ // grid location
					16, 0
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelJustifyRight
			},
			{ // next size step 5; font_Vx8; no seconds
				{ // minimum dimensions
					25, 8
				},
				{ // grid location
					16, 0
				},
				// flags
				duds::ui::graphics::GridLayoutConfig::PanelJustifyRight
			},
		}
		// flags (default)
	};
	/**
	 * An image for each size-step. They are rendered as required and kept until
	 * the time changes so they can be re-used.
	 */
	duds::ui::graphics::BppImageSptr outputs[5];
	/**
	 * The start time, in IST, of the next second. Used for checks to see if
	 * the time should be rendered.
	 */
	duds::time::interstellar::SecondTime nextSec;
	/**
	 * The time to render in the local time zone.
	 */
	std::tm time;
	bool blink = false;
	struct Token { };
public:
	TimePanel(Token);
	static std::shared_ptr<TimePanel> make() {
		return std::make_shared<TimePanel>(Token());
	}
	void updateTime(const duds::data::Measurement::TimeSample &ts);
	const duds::ui::graphics::GridLayoutConfig &suggestedLayout() const {
		return glc;
	}
	virtual const duds::ui::graphics::BppImage *render(
		duds::ui::graphics::ImageLocation &offset,
		duds::ui::graphics::ImageDimensions &dim,
		duds::ui::graphics::PanelMargins &margin,
		int sizeStep
	);
};

typedef std::shared_ptr<TimePanel>  TimePanelSptr;
