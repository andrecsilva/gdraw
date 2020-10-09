#pragma once

namespace gdraw{

struct coord_t{
	double x;
	double y;

	coord_t() {x=0;y=0;}

	coord_t(double _x, double _y) : x(_x), y(_y) {}


	friend std::ostream& operator<<(std::ostream& out, const coord_t& c);
};

std::ostream& operator<<(std::ostream& out, const coord_t& c){
	std::cout.precision(5);
	out << c.x << "," << c.y;
	return out;
}

struct cubicSpline{
	coord_t from;
	coord_t control1;
	coord_t control2;
	coord_t to;

	cubicSpline(){}

	cubicSpline(coord_t _from, coord_t _control1, coord_t _control2, coord_t _to) : from(_from), control1(_control1), control2(_control2), to(_to) {}

	friend std::ostream& operator<<(std::ostream& out, const cubicSpline& spline);
};

std::ostream& operator<<(std::ostream& out, const cubicSpline& spline){
	std::cout.precision(5);
	out << spline.from << " " << spline.control1 << " " << spline.control2 << " " << spline.to;
	return out;
}

}//namespace
