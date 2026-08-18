# EB2::Build(makeShop(SphereIF), Vector<Geometry>, ...) builds every coarse
# level directly with the marching-cubes generator instead of coarsening.
nx = 64
ny = 64
nz = 64

max_grid_size = 32
algorithm_tests = 0
api_build = sphere
api_all_levels = 1
required_coarsening_level = 2
max_coarsening_level = 2

eb2.geometry_method = marching_cubes
eb2.sphere_center = 0.0 0.0 0.0
eb2.sphere_radius = 0.7
eb2.sphere_has_fluid_inside = 0
