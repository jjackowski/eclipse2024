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
#include "TextPanel.hpp"

void TextPanel::text(const std::string &t) {
	if (t != txt) {
		txt = t;
		img.reset();
	}
}

const duds::ui::graphics::BppImage *TextPanel::render(
	duds::ui::graphics::ImageLocation &offset,
	duds::ui::graphics::ImageDimensions &dim,
	duds::ui::graphics::PanelMargins &margin,
	int sizeStep
) {
	if (txt.empty()) {
		return nullptr;
	}
	if (!img) {
		img = strcache->text(txt);
	}
	std::int16_t w = img->dimensions().w;
	dim = img->dimensions().minExtent(dim);
	if (dim.w == w) {
		// trim last pixel; usually empty
		--dim.w;
	}
	return img.get();
}
