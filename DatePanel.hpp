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
#include <boost/date_time/gregorian/gregorian.hpp>

// not thread-safe
class DatePanel : public duds::ui::graphics::Panel {
	/**
	 * Layout configuration data, mostly size steps.
	 * Test case: Wed Oct 28, 2023
	 * | Font       | Without year | With Year
	 * |------------|-------------:|----------:
	 * | font_Vx7   | 43           | 66
	 * | font_Vx8   | 51           | 79
	 * | font_Vx8B  | 76           | 120
	 * | font_10x18 | 100          | 160
	 */
	duds::ui::graphics::GridLayoutConfig glc = {
		{ // sizes
			/*
			{ // size step 0; font_Vx8B with year
				{ // minimum dimensions
					120, 8
				},
				{ // grid location
					0, 0
				}
				// flags (default)
			},
			{ // next size step; font_Vx8 with year
				{ // minimum dimensions
					79, 8
				},
				{ // grid location
					0, 0
				}
				// flags (default)
			},
			{ // next size step; font_Vx8B without year
				{ // minimum dimensions
					76, 8
				},
				{ // grid location
					0, 0
				},
				// flags; don't show unless configured to never show the year
				//duds::ui::graphics::GridLayoutConfig::PanelHidden
			},
			*/
			{ // next size step; font_Vx8 without year
				{ // minimum dimensions
					51, 8
				},
				{ // grid location
					0, 0
				}
				// flags (default)
			}
		},
		duds::ui::graphics::GridLayoutConfig::PanelJustifyLeft |
		duds::ui::graphics::GridLayoutConfig::PanelWidthExpand
	};
	/**
	 * An image for each size-step. They are rendered as required and kept until
	 * the date changes so they can be re-used.
	 */
	duds::ui::graphics::BppImageSptr outputs[2];
	/**
	 * Used to produce the date string and placed here to avoid remaking the
	 * string stream's buffer and a boost::gregorian::date_facet every time the
	 * string is made.
	 */
	std::ostringstream oss;
	/**
	 * The current calendar date.
	 */
	boost::gregorian::date date;
	/**
	 * The date as a string.
	 */
	std::string dateStr;
	/**
	 * The start time, in IST, of the next day in the local time zone. The type
	 * is NanoTime to avoid a type conversion to make comparisons easier.
	 */
	duds::time::interstellar::NanoTime nextDay;
	/**
	 * Widths of images in @a outputs with year removed.
	 */
	std::int16_t widths[2];
	struct Token { };
public:
	DatePanel(Token);
	static std::shared_ptr<DatePanel> make() {
		return std::make_shared<DatePanel>(Token());
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

typedef std::shared_ptr<DatePanel>  DatePanelSptr;
