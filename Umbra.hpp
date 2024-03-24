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
#include <gdal/ogrsf_frmts.h>
#include <memory>
#include <boost/exception/info.hpp>
#include <boost/utility.hpp>

struct GDALDatasetDeleter {
	void operator()(GDALDataset *ds) {
		GDALClose(ds);
	}
};

typedef std::unique_ptr<GDALDataset, GDALDatasetDeleter>  GDALDatasetUPtr;
typedef std::shared_ptr<GDALDataset>  GDALDatasetSPtr;

struct OGRFeatureDeleter {
	void operator()(OGRFeature *f) {
		OGRFeature::DestroyFeature(f);
	}
};

typedef std::unique_ptr<OGRFeature, OGRFeatureDeleter>  OGRFeatureUPtr;

struct UmbraError : virtual std::exception, virtual boost::exception { };
struct UmbraOpenError : UmbraError { };
struct UmbraNoLayer : UmbraError { };
typedef boost::error_info<struct Info_LayerName, std::string>  UmbraLayerName;
struct UmbraNoFeature : UmbraError { };
typedef boost::error_info<struct Info_FeatureIndex, GIntBig>  UmbraFeatureIndex;

/**
 * Processes umbra shapes from NASA to determine if a location will see the
 * total eclipse of August 21, 2017, and if so, when the total eclipse will
 * begin and end. No attempt is made at interpolation between shapes, so the
 * start time may be as much as almost a second late, and the end time may be
 * as much as almost a second early. I'm new to using GIS software, so I'm not
 * sure how best to implement useful interpolation.
 * @author  Jeff Jackowski
 */
class Umbra : boost::noncopyable {
	GDALDatasetUPtr dataset;
	GIntBig total, first, last;
	OGRLayer *umbras;
	OGRFeatureDefn *featdef;
	int startT, endT; // in seconds from start of day UTC -- same as in shapefile
	double poslon, poslat;
	bool verbose;
public:
	/**
	 * @param fname  The name of the shapefile with the umbra shapes. It
	 *               should be umbra_hi.shp, but could include a more complete
	 *               path. See https://svs.gsfc.nasa.gov/5073 for the files.
	 * @param v      True for verbose output to stdout.
	 */
	Umbra(const std::string &fname, bool v = false);
	/**
	 * Finds if the given location is within any of the umbra shapes, and
	 * returns true if it is.
	 */
	bool check(double lon, double lat);
	/**
	 * In seconds from midnight, UTC, day of eclipse.
	 */
	int startTime() const {
		return startT;
	}
	/**
	 * In seconds from midnight, UTC, day of eclipse.
	 */
	int endTime() const {
		return endT;
	}
};
