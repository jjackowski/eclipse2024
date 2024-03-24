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
#include <duds/ui/graphics/Panel.hpp>
#include <duds/ui/graphics/BppStringCache.hpp>

class TextPanel : public duds::ui::graphics::Panel {
	/**
	 * The string cache to use for rendering text.
	 */
	duds::ui::graphics::BppStringCacheSptr strcache;
	/**
	 * Rendered text ready for display, or empty.
	 */
	duds::ui::graphics::ConstBppImageSptr img;
	/**
	 * The string to render.
	 */
	std::string txt;
	struct Token { };
public:
	TextPanel(const duds::ui::graphics::BppStringCacheSptr &bsc, Token) :
		strcache(bsc) { }
	static std::shared_ptr<TextPanel> make(
		const duds::ui::graphics::BppStringCacheSptr &bsc
	) {
		return std::make_shared<TextPanel>(bsc, Token());
	}
	const std::string &text() const {
		return txt;
	}
	void text(const std::string &t);
	virtual const duds::ui::graphics::BppImage *render(
		duds::ui::graphics::ImageLocation &offset,
		duds::ui::graphics::ImageDimensions &dim,
		duds::ui::graphics::PanelMargins &margin,
		int sizeStep
	);
};

typedef std::shared_ptr<TextPanel>  TextPanelSptr;
