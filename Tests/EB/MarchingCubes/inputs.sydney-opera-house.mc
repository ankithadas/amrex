nx = 256
ny = 128
nz = 64

max_grid_size = 32

algorithm_tests = 0
custom_stl_test = 1

geometry.prob_lo = -0.32 -0.16 -0.08
geometry.prob_hi =  0.32  0.16  0.08

eb2.stl_geometry_method = marching_cubes
eb2.stl_file = SydneyOperaHouseClean.stl
eb2.small_volfrac = 1.e-12
eb2.stl_scale = 1.e-2
eb2.stl_center = -0.2443869209 -0.0039359283 -0.0678719997
eb2.cover_multiple_cuts = 1
eb2.mc_stl_file = SydneyOperaHouseClean_mc_triangles.stl
eb2.eb_surface_stl_file = SydneyOperaHouseClean_mc.stl
