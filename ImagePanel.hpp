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
#include <duds/ui/graphics/GridLayoutConfig.hpp>
#include <duds/ui/graphics/BppStringCache.hpp>

/**
 * Renders an image on a Panel.
 * @author  Jeff Jackowski.
 */
class ImagePanel : public duds::ui::graphics::Panel {
	/**
	 * Image to show.
	 */
	duds::ui::graphics::ConstBppImageSptr img;
	struct Token { };
public:
	ImagePanel(Token) { }
	ImagePanel(const duds::ui::graphics::ConstBppImageSptr &biptr, Token) :
		img(biptr) { }
	static std::shared_ptr<ImagePanel> make() {
		return std::make_shared<ImagePanel>(Token());
	}
	static std::shared_ptr<ImagePanel> make(
		const duds::ui::graphics::ConstBppImageSptr &biptr
	) {
		return std::make_shared<ImagePanel>(biptr, Token());
	}
	const duds::ui::graphics::ConstBppImageSptr &image() const {
		return img;
	}
	void image(const duds::ui::graphics::ConstBppImageSptr &bi) {
		img = bi;
	}
	virtual const duds::ui::graphics::BppImage *render(
		duds::ui::graphics::ImageLocation &offset,
		duds::ui::graphics::ImageDimensions &dim,
		duds::ui::graphics::PanelMargins &margin,
		int sizeStep
	);
};

typedef std::shared_ptr<ImagePanel>  ImagePanelSptr;
