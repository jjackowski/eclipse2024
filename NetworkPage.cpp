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
#include "NetworkPage.hpp"
#include "Screen.hpp"
#include <linux/wireless.h>

NetInterface::NetInterface(const std::string &n, const sockaddr_in *sa) :
ifname(n),
addr(boost::asio::ip::address_v4(
	htonl(sa->sin_addr.s_addr)
	// sin_addr is allways big endian; must not use int constructor since it
	// expects host byte order
	//(boost::asio::ip::address_v4::bytes_type*)&(sa->sin_addr.s_addr)
)) {
	wlQuery();
}

NetInterface::NetInterface(
	const std::string &n, const boost::asio::ip::address_v4 &sa
) : ifname(n), addr(sa) {
	wlQuery();
}

void NetInterface::wlQuery() {
	// query for wireless network data
	// request struct
	iwreq req;
	// dummy socket
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	// copy in network interface name
	std::copy_n(
		ifname.begin(),
		std::min(ifname.size() + 1, sizeof(req.ifr_name)),
		req.ifr_name
	);
	// ask for network name
	char essidbuff[32];
	req.u.essid.pointer = essidbuff;
	req.u.essid.length = 32;
	/**
	 * @bug  Always fails on my development machine (kernel 6.6.13), but
	 *       succeeds on the target Raspberry Pi (kernel 6.1.?). Why?
	 */
	if (ioctl(sock, SIOCGIWESSID, &req) >= 0) {
		// reported string is not NULL terminated, but length is reported
		id.append(essidbuff, req.u.essid.length);
	}
	close(sock);
}

Page::SelectionResponse NetworkPage::select(
	const DisplayInfo &di,
	SelectionCause sc
) {
	// if change is already non-zero, keep it non-zero
	change |= fillnetifs();
	if (
		// user wants it
		(sc == SelectUser) ||
		(
			// networks present
			!netifs.empty() &&
			// don't auto-show once the eclipse starts
			(di.goodfix && (di.now < (di.start - DisplayInfo::beforeTotality)))
		)
	) {
		return SelectPage;
	}
	return SkipPage;
}

int NetworkPage::fillnetifs() {
	std::set<std::string> seen;
	ifaddrs *ifAddrStruct = nullptr;
	ifaddrs *ifa = nullptr;
	getifaddrs(&ifAddrStruct);
	int updates = 0;

	for (ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		// IPv4 address?
		if (ifa->ifa_addr->sa_family == AF_INET) {
			boost::asio::ip::address_v4 ip4(
				htonl(((sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr)
				//(boost::asio::ip::address_v4::bytes_type)
				//&((sockaddr_in*)ifa->ifa_addr)->sin_addr
			);
			if (!ip4.is_loopback() && !ip4.is_multicast()) {

				//std::cout << "Net: " << ifa->ifa_name << "  addr: " <<
				//ip4.to_string() << std::endl;

				// Test system has multiple addresses per adapter; only the
				// first is reported by ifconfig, so ignore the rest. KDE shows
				// the other address. Neither shows both addresses. 
				// One day figure out what is going on.
				if (seen.count(ifa->ifa_name) > 0) {
					continue;
				}
				// look for an existing network
				NetInterfaceSet::iterator iter = netifs.find(ifa->ifa_name);
				// found it?
				if (iter != netifs.end()) {
					// different address?
					if (iter->address() != ip4) {
						// replace the object
						netifs.erase(iter);
						netifs.emplace(ifa->ifa_name, ip4);
						++updates;
					}
					// if same address, leave object as is
				} else {
					// new network, new object
					netifs.emplace(ifa->ifa_name, ip4);
					++updates;
				}
				// always mark as seen
				seen.insert(ifa->ifa_name);
			}
		}
	}
	if (ifAddrStruct) {
		freeifaddrs(ifAddrStruct);
	}
	// check for removals
	NetInterfaceSet::iterator iter = netifs.begin();
	while (iter != netifs.end()) {
		std::set<std::string>::iterator siter = seen.find(iter->name());
		// not found?
		if (siter == seen.end()) {
			// remove it
			iter = netifs.erase(iter);
			++updates;
		} else {
			++iter;
		}
	}
	return updates;
}

void NetworkPage::show(const DisplayInfo &, Screen *scr) {
	scr->showTitle("Network");
	scr->largeTime(false);
	if (change) {
		last = netifs.begin();
	} else if (netifs.empty()) {
		scr->showText("No networks", 0, 0);
		return;
	} else {
		++last;
		if (last == netifs.end()) {
			last = netifs.begin();
		}
	}
	change = 0;
	std::ostringstream oss;
	oss << last->name();
	scr->showText(oss.str(), 0, 0);
	oss.str(std::string());
	if (last->isWireless()) {
		oss << last->essid();
		scr->showText(oss.str(), 0, 1);
		oss.str(std::string());
	}
	oss << last->address().to_string();
	scr->showText(oss.str(), 0, 2);
}

void NetworkPage::update(const DisplayInfo &, Screen *) { }

void NetworkPage::hide(const DisplayInfo &, Screen *scr) {
	scr->hideText();
	scr->largeTime(true);
}
