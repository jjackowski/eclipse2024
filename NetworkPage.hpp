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
#include <duds/ui/menu/renderers/BppMenuRenderer.hpp>
#include <boost/asio/ip/address.hpp>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Page.hpp"
#include <set>

/**
 * Details an IPv4 network interface.
 * Wireless network reporting is unreliable; it may not provide the name of the
 * LAN and then will not report the LAN as wireless. The address will still be
 * reported,
 * @author  Jeff Jackowski
 */
class NetInterface {
	/**
	 * The network address.
	 */
	boost::asio::ip::address addr;
	/**
	 * The interface name.
	 */
	std::string ifname;
	/**
	 * The wireless LAN name; empty if not wireless or not reported.
	 */
	std::string id;
	void wlQuery();
public:
	NetInterface() = delete;
	NetInterface(const std::string &n, const sockaddr_in *sa);
	NetInterface(const std::string &n, const boost::asio::ip::address_v4 &sa);
	const boost::asio::ip::address &address() const {
		return addr;
	}
	const std::string &name() const {
		return ifname;
	}
	const std::string &essid() const {
		return id;
	}
	/**
	 * @bug  Some systems, like my primary development machine, will not report
	 *       the name of the wireless LAN and cause this function to always
	 *       return false.
	 */
	bool isWireless() const {
		return !id.empty();
	}
	bool operator < (const NetInterface &ni) const {
		return ifname < ni.name();
	}
	bool operator < (const std::string &ni) const {
		return ifname < ni;
	}
};

/**
 * Compares and orders a NetInterface  with strings.
 */
inline bool operator < (const std::string &name, const NetInterface &ni) {
	return name < ni.name();
}

/**
 * Performs a less-than comparison on two objects of any types that can be
 * compared in this way.
 */
struct GenericTransparentComp {
	typedef int is_transparent;
	template <class A, class B>
	bool operator()(const A &a, const B &b) const {
		return a < b;
	}
};

/**
 * A set of NetInterface objects that can be found by searching for interface
 * names.
 */
typedef std::set<NetInterface, GenericTransparentComp>  NetInterfaceSet;

/**
 * A page that shows the user a network interface and its address. Each time
 * the page is shown, it cycles to the next network interface. Not shown unless
 * requested, or before the eclipse, and if there are no network interfaces up.
 * @author  Jeff Jackowski
 */
class NetworkPage : public Page {
	NetInterfaceSet netifs;
	NetInterfaceSet::iterator last;
	int fillnetifs();
	int change = 0;
public:
	//NetworkPage();
	virtual Page::SelectionResponse select(
		const DisplayInfo &,
		Page::SelectionCause
	);
	virtual void show(const DisplayInfo &di, Screen *scr);
	virtual void hide(const DisplayInfo &di, Screen *scr);
	virtual void update(const DisplayInfo &di, Screen *scr);
};
