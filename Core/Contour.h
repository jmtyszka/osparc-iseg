/*
 * Copyright (c) 2018 The Foundation for Research on Information Technologies in Society (IT'IS).
 * 
 * This file is part of iSEG
 * (see https://github.com/ITISFoundation/osparc-iseg).
 * 
 * This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 */
#pragma once

#include "iSegCore.h"

#include "Data/Point.h"

#include <iostream>
#include <vector>

namespace iseg {

class ISEG_CORE_API Contour
{
public:
	Contour();
	Contour(std::vector<Point>* Pt_vec);

	void add_point(Point p);
	void add_points(std::vector<Point>* Pt_vec);
	void print_contour();
	void doug_peuck(float epsilon, bool closed);
	void presimplify(float d, bool closed);
	unsigned int return_n();
	void return_contour(std::vector<Point>* Pt_vec);
	void clear();

private:
	unsigned int n;
	std::vector<Point> plist;
	void doug_peuck_sub(float epsilon, const unsigned int p1,
						const unsigned int p2, std::vector<bool>* v1_p);
};

class ISEG_CORE_API Contour2
{
public:
	//abcd void doug_peuck(float epsilon,vector<Point> *Pt_vec,vector<unsigned short> *Meetings_vec,vector<Point> *Result_vec);
	void doug_peuck(float epsilon, std::vector<Point>* Pt_vec,
					std::vector<unsigned>* Meetings_vec,
					std::vector<Point>* Result_vec);

private:
	unsigned int n;
	unsigned int m;
	void doug_peuck_sub(float epsilon, std::vector<Point>* Pt_vec,
						unsigned short p1, unsigned short p2,
						std::vector<bool>* v1_p);
	void doug_peuck_sub2(float epsilon, std::vector<Point>* Pt_vec,
						 unsigned short p1, unsigned short p2,
						 std::vector<bool>* v1_p);
};
} // namespace iseg
