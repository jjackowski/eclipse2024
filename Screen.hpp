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
#include <duds/ui/graphics/BppImageArchive.hpp>
#include "DatePanel.hpp"
#include "ImagePanel.hpp"
#include "TimePanel.hpp"
#include "TextPanel.hpp"

/**
 * Handles layout and panels rendered to the graphic display. The pages of this
 * application go through excactly one of these objects for output.
 * @author  Jeff Jackowski
 */
class Screen : boost::noncopyable {
	/**
	 * Handles dynamic layout for the screen.
	 */
	duds::ui::graphics::PriorityGridLayout pgl;
	/*
	 * Output display.
	 */
	//duds::hardware::display::BppGraphicDisplaySptr disp;
	DatePanelSptr dateP;
	TimePanelSptr timeP;
	TextPanelSptr textP[4][3];
	TextPanelSptr titleP, infoP;
	ImagePanelSptr warnP, sepP, pinP, menuP;
	bool doLayout = false;
	/**
	 * Changes the layout flags. If the flag value changed, sets @a doLayout
	 * and returns true.
	 */
	bool changeLayoutFlags(
		int pri,
		duds::ui::graphics::GridLayoutConfig::Flags value,
		duds::ui::graphics::GridLayoutConfig::Flags mask
	);
public:
	enum PanelPriorities {
		PanelWarningPriority = 1,
		PanelPinPriority,
		PanelSpacerR0Priority,
		PanelSpacerR1Priority,
		PanelSpacerR2Priority,
		PanelTextC0R0Priority,
		PanelTextC0R1Priority,
		PanelTextC0R2Priority,
		PanelTextC1R0Priority,
		PanelTextC1R1Priority,
		PanelTextC1R2Priority,
		PanelTextC2R0Priority,
		PanelTextC2R1Priority,
		PanelTextC2R2Priority,
		PanelTextC3R0Priority,
		PanelTextC3R1Priority,
		PanelTextC3R2Priority,
		PanelTitlePriority,
		PanelMenuPriority,
		PanelStatusPriority,  // not yet used
		PanelInfoMsgPriority,
		PanelTimePriority,
		PanelDatePriority,
	};
	Screen(
		const duds::ui::graphics::BppImageArchiveSptr &iconArc,
		const duds::ui::graphics::ImageDimensions &dim
	);
	void render(
		duds::ui::graphics::BppImage &image,
		const duds::data::Measurement::TimeSample &time
	);
	duds::ui::graphics::ImageDimensions menuSize() const {
		return duds::ui::graphics::ImageDimensions(
			pgl.renderFill().w,
			pgl.renderFill().h - 8
		);
	}
	void largeTime(bool allow);
	void showTitle(const std::string &t);
	void hideTitle();
	duds::ui::graphics::ImageDimensions titleSize() const;
	void showInfoMsg(const std::string &m);
	void hideInfoMsg();
	void showText(const std::string &t, int c, int r);
	void hideText(int c, int r);
	void hideText(); // hides all text
	void showPin();
	void hidePin();
	void showMenu(const duds::ui::graphics::ConstBppImageSptr &img);
	void hideMenu();
};
