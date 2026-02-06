#include "schema.hh"
#include "tsdb.hh"

#include <cstring>

using namespace schema;

struct Vec3 {
    f64 x;
    f64 y;
    f64 z;
};


auto main() -> i32 {
    TypeRegistry reg;

    // Create the Schema
    auto vec3_handle = reg.register_struct({
        {"x", TypeRegistry::F64},
        {"y", TypeRegistry::F64},
        {"z", TypeRegistry::F64},
    });

    // Init DB
    TSDB db { reg };

    // Insert some type into the DB
    auto vec3 = Vec3 {
        .x = 1,
        .y = 1,
        .z = 1
    };
    db.insert(reinterpret_cast<std::byte*>(&vec3), vec3_handle); // maybe make it just pass in (vec3, vec3_handle)
       
    // Query that type
    Vec3 new_vec;
    db.query_all_into(vec3_handle, reinterpret_cast<std::byte*>(&new_vec));
    std::println("new_vec {{.x = {}, .y = {}, .z = {}}}", new_vec.x, new_vec.y, new_vec.z);
    
    return 0;
}
