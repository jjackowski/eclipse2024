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
#ifndef PAGE_HPP_
#define PAGE_HPP_

#include <boost/noncopyable.hpp>
#include "DisplayStuff.hpp"

class Screen;

/**
 * An interface to use for displayable pages.
 */
class Page : boost::noncopyable {
public:
	virtual ~Page() { };
	/**
	 * Ways a page may be selected for display.
	 */
	enum SelectionCause {
		/**
		 * Page selection by automatic cycling.
		 */
		SelectAuto,
		/**
		 * Page selection by user input. Some pages that may be skipped over
		 * by automatic cycling can still be selected by the user.
		 */
		SelectUser
	};
	/**
	 * Responses to a page selection.
	 */
	enum SelectionResponse {
		/**
		 * The page accepts the selection.
		 */
		SelectPage,
		/**
		 * The page will be skipped over.16
		 */
		SkipPage
	};
	/**
	 * Handles a request to select the page for display. The page will not be
	 * shown if SkipPage is returned. If SelectPage will be returned, the
	 * function must @b not configure the screen.
	 */
	virtual SelectionResponse select(
		const DisplayInfo &di,
		SelectionCause cause
	) = 0;
	/**
	 * Configures the screen to show the page when it was not previously
	 * visible. update() will be called immediately after this call.
	 */
	virtual void show(const DisplayInfo &di, Screen *scr) = 0;
	/**
	 * Configures the screen to hide what was used by this Page.
	 */
	virtual void hide(const DisplayInfo &di, Screen *scr) = 0;
	/**
	 * Updates the page on the display; called normally once per second.
	 */
	virtual void update(const DisplayInfo &di, Screen *scr) { };
};

#endif        //  #ifndef PAGE_HPP
