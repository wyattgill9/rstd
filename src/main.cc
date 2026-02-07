#include "tsdb.hh"

#include <cstring>

struct Vec3 {
    f64 x, y, z;
};

auto main() -> i32 {
	TSDB db {4};

	auto vec3_handle = db.register_struct(
    "Vec3", {
		{"x", TSDB::F64},
		{"y", TSDB::F64},
		{"z", TSDB::F64},
	});

	db.insert(Vec3 { .x = 1, .y = 1, .z = 1}, vec3_handle);

	Vec3 new_vec;
	db.query_first(new_vec, vec3_handle);
    std::println("new_vec {{.x = {}, .y = {}, .z = {}}}", new_vec.x, new_vec.y, new_vec.z);

	return 0;
}
