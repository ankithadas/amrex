nx = 128
ny = 128
nz = 32

nx = 256
ny = 256
nz = 64

max_grid_size = 32

algorithm_tests = 0
custom_stl_test = 1

geometry.prob_lo = -0.50 -0.40  0.00
geometry.prob_hi =  0.50  0.60  0.25

eb2.stl_geometry_method = marching_cubes
# eb2.stl_geometry_method = legacy
eb2.stl_file = B1-5.stl
# eb2.small_volfrac = 1.e-12
eb2.stl_scale = 1e-3
eb2.stl_center = 0.0 0.0 0.0
eb2.cover_multiple_cuts = 1
eb2.mc_stl_file = B1-5_mc_triangles.stl
eb2.eb_surface_stl_file = B1-5_mc.stl
# eb2.eb_surface_stl_file = B1-5_legacy.stl
