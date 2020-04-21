#include "types.h"

std::ostream& operator<<(std::ostream& ios, const Vec3& rhs) {
	return ios << rhs.x_ << ' ' << rhs.y_ << ' ' << rhs.z_;
}

std::ostream& operator<<(std::ostream& ios, const Vec4& rhs)
{
	return ios << rhs.x_ << ' ' << rhs.y_ << ' ' << rhs.z_ << ' ' <<rhs.w_;
}
