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
#include <duds/ui/graphics/BppFontPool.hpp>
#include "Screen.hpp"

extern duds::ui::graphics::BppFontPool FontPool;

Screen::Screen(
	const duds::ui::graphics::BppImageArchiveSptr &iconArc,
	const duds::ui::graphics::ImageDimensions &dim
) :
	/* disp(d),*/ dateP(DatePanel::make()), timeP(TimePanel::make()),
	titleP(TextPanel::make(FontPool.getStringCache("Title"))),
	infoP(TextPanel::make(FontPool.getStringCache("Text"))),
	warnP(ImagePanel::make(iconArc->get("Warning"))),
	sepP(ImagePanel::make(iconArc->get("Separator"))),
	pinP(ImagePanel::make(iconArc->get("Pin"))),
	menuP(ImagePanel::make())
{
	pgl.renderFill(dim);
	pgl.add(dateP, dateP->suggestedLayout(), PanelDatePriority);
	pgl.add(timeP, timeP->suggestedLayout(), PanelTimePriority);
	pgl.add(warnP, duds::ui::graphics::GridLayoutConfig {
		{ // sizes
			{ // a size-step
				warnP->image()->dimensions(),
				{
					15, 0
				}
			}
		},
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	}, PanelWarningPriority);
	pgl.add(pinP, duds::ui::graphics::GridLayoutConfig {
		{ // sizes
			{ // a size-step
				pinP->image()->dimensions(),
				{
					14, 0
				}
			}
		},
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	}, PanelPinPriority);
	pgl.add(menuP, duds::ui::graphics::GridLayoutConfig {
		{ // sizes
			{ // a size-step
				{
					dim.w, dim.h - 8
				},
				{
					0, 1
				}
			}
		},
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	}, PanelMenuPriority);
	//pgl.panelConfig(PanelWarningPriority).hide();
	// start with title layout
	duds::ui::graphics::GridLayoutConfig titleGlc = {
		{
			{
				{ // minimum dimensions
					8 * 8, 8
				},
				{ // grid location
					0, 0
				}
			}
		},
		// flags (default)
		duds::ui::graphics::GridLayoutConfig::PanelHidden |
		duds::ui::graphics::GridLayoutConfig::PanelWidthExpand
	};
	pgl.add(titleP, titleGlc, PanelTitlePriority);
	// text spots, 4 columns, 3 rows
	int pri = PanelTextC0R0Priority;
	for (int c = 0; c < 4; ++c) {
		for (int r = 0; r < 3; ++r) {
			textP[c][r] = TextPanel::make(FontPool.getStringCache("Text"));
			// columns 0 & 2 for titles; will have at least 3 chars & expand
			// columns 1 & 3 for numbers up to 5 chars; has space for separation
			duds::ui::graphics::GridLayoutConfig textGlc = {
				{
					{
						{ // minimum dimensions
							(c & 1) ? ((c > 2) ? (5 * 8) : (9 * 8)) : (3 * 8 + 4), 8
						},
						{ // grid location
							(c > 1) ? (c + 1) : c, r + 1
						}
					}
				},
				// flags (default)
				duds::ui::graphics::GridLayoutConfig::PanelHidden |
				((c & 1) ?
					duds::ui::graphics::GridLayoutConfig::PanelJustifyRight :
					(
						duds::ui::graphics::GridLayoutConfig::PanelJustifyLeft |
						duds::ui::graphics::GridLayoutConfig::PanelWidthExpand
					)
				)
			};
			pgl.add(textP[c][r], textGlc, pri++);
		}
	}
	// text spacers
	for (int r = 0; r < 3; ++r) {
		pgl.add(
			sepP,
			{
				{
					{
						{ // minimum dimensions
							6, 8
						},
						{ // grid location
							2, r + 1
						}
					}
				},
				duds::ui::graphics::GridLayoutConfig::PanelHidden |
				duds::ui::graphics::GridLayoutConfig::PanelCenter
			},
			PanelSpacerR0Priority + r
		);
	}
	// end with info message layout
	duds::ui::graphics::GridLayoutConfig infoGlc = {
		{
			{
				{ // minimum dimensions
					dim.w, 8
				},
				{ // grid location
					0, 4
				},
			}
		},
		// flags (default)
		duds::ui::graphics::GridLayoutConfig::PanelHidden |
		duds::ui::graphics::GridLayoutConfig::PanelCenter
	};
	pgl.add(infoP, infoGlc, PanelInfoMsgPriority);
	pgl.layout();
}

void Screen::render(
	duds::ui::graphics::BppImage &image,
	const duds::data::Measurement::TimeSample &time
) {
	if (doLayout) {
		// check for need to expand or collapse text fields
		for (int r = 0; r < 3; ++r) {
			if (pgl.panelConfig(PanelTextC0R0Priority + r + 2 * 3).flags &
				duds::ui::graphics::GridLayoutConfig::PanelHidden
			) {
				// expand text field in second column
				pgl.panelConfig(PanelTextC0R0Priority + r + 1 * 3).
					sizes[0].minDim.w = 9 * 8;
			} else {
				// collapse field in second column
				pgl.panelConfig(PanelTextC0R0Priority + r + 1 * 3).
					sizes[0].minDim.w = 5 * 8;
			}
		}
		pgl.layout();
		doLayout = false;
	}
	image.clearImage();
	dateP->updateTime(time);
	timeP->updateTime(time);
	pgl.render(image);
}

bool Screen::changeLayoutFlags(
	int pri,
	duds::ui::graphics::GridLayoutConfig::Flags value,
	duds::ui::graphics::GridLayoutConfig::Flags mask
) {
	duds::ui::graphics::GridLayoutConfig &glc = pgl.panelConfig(pri);
	duds::ui::graphics::GridLayoutConfig::Flags old = glc.flags;
	glc.flags.setMasked(value, mask);
	if (old == glc.flags) {
		return false;
	} else {
		doLayout = true;
		return true;
	}
}

void Screen::largeTime(bool allow) {
	duds::ui::graphics::GridLayoutConfig &glc = pgl.panelConfig(PanelTimePriority);
	for (auto &step : glc.sizes) {
		if (step.loc == duds::ui::graphics::GridLocation(1, 1)) {
			duds::ui::graphics::GridLayoutConfig::Flags old = step.flags;
			step.flags.setMasked(
				allow ?
					duds::ui::graphics::GridLayoutConfig::PanelShown :
					duds::ui::graphics::GridLayoutConfig::PanelHidden,
				duds::ui::graphics::GridLayoutConfig::PanelHidden
			);
			if (old != step.flags) {
				doLayout = true;
			}
		}
	}
}

void Screen::showTitle(const std::string &t) {
	titleP->text(t);
	changeLayoutFlags(
		PanelTitlePriority,
		duds::ui::graphics::GridLayoutConfig::PanelShown,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}

void Screen::hideTitle() {
	//pgl.panelConfig(PanelTitlePriority).hide();
	changeLayoutFlags(
		PanelTitlePriority,
		duds::ui::graphics::GridLayoutConfig::PanelHidden,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}

duds::ui::graphics::ImageDimensions Screen::titleSize() const {
	return pgl.layoutDimensions(PanelTitlePriority);
}

void Screen::showInfoMsg(const std::string &m) {
	infoP->text(m);
	if (
		changeLayoutFlags(
			PanelInfoMsgPriority,
			duds::ui::graphics::GridLayoutConfig::PanelShown,
			duds::ui::graphics::GridLayoutConfig::PanelHidden
		)
	) {
		pgl.panelConfig(PanelWarningPriority).show();
	}
}

void Screen::hideInfoMsg() {
	if (
		changeLayoutFlags(
			PanelInfoMsgPriority,
			duds::ui::graphics::GridLayoutConfig::PanelHidden,
			duds::ui::graphics::GridLayoutConfig::PanelHidden
		)
	) {
		pgl.panelConfig(PanelWarningPriority).hide();
	}
}

void Screen::showText(const std::string &t, int c, int r) {
	assert((c >= 0) && (c < 4) && (r >= 0) && (r < 3));
	textP[c][r]->text(t);
	if (
		changeLayoutFlags(
			PanelTextC0R0Priority + r + c * 3,
			duds::ui::graphics::GridLayoutConfig::PanelShown,
			duds::ui::graphics::GridLayoutConfig::PanelHidden
		) && (c > 1)
	) {
		pgl.panelConfig(PanelSpacerR0Priority + r).show();
	}
}

void Screen::hideText(int c, int r) {
	assert((c >= 0) && (c < 4) && (r >= 0) && (r < 3));
	if (
		changeLayoutFlags(
			PanelTextC0R0Priority + r + c * 3,
			duds::ui::graphics::GridLayoutConfig::PanelHidden,
			duds::ui::graphics::GridLayoutConfig::PanelHidden
		) && (c > 1)
	) {
		pgl.panelConfig(PanelSpacerR0Priority + r).hide();
	}
}

// hides all text
void Screen::hideText() {
	for (int pri = PanelSpacerR0Priority; pri < PanelTextC3R2Priority + 1; ++pri) {
		pgl.panelConfig(pri).hide();
	}
	doLayout = false;
}

void Screen::showPin() {
	changeLayoutFlags(
		PanelPinPriority,
		duds::ui::graphics::GridLayoutConfig::PanelShown,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}

void Screen::hidePin() {
	changeLayoutFlags(
		PanelPinPriority,
		duds::ui::graphics::GridLayoutConfig::PanelHidden,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}

void Screen::showMenu(const duds::ui::graphics::ConstBppImageSptr &img) {
	menuP->image(img);
	changeLayoutFlags(
		PanelMenuPriority,
		duds::ui::graphics::GridLayoutConfig::PanelShown,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}

void Screen::hideMenu() {
	changeLayoutFlags(
		PanelMenuPriority,
		duds::ui::graphics::GridLayoutConfig::PanelHidden,
		duds::ui::graphics::GridLayoutConfig::PanelHidden
	);
}
