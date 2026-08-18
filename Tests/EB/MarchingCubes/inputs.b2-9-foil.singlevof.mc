# B2-9 hull as a multi-VoF (multi-cut) marching-cubes EB case.
# Same geometry and domain as inputs.b2-9-foil.mc on marching-cubes-eb-plan;
# here EB2::BuildMultiValuedMultiCut retains multi-cut cells instead of
# covering them, so small_volfrac = 0 and cover_multiple_cuts is left off.
#
#   hull bbox : +/-10.374242  +/-2.536116  +/-1.861793  m
#   domain    : 22.0 x 6.0 x 4.5 m, dx set by nx/ny/nz below

nx = 704
ny = 192
nz = 144

max_grid_size = 32

algorithm_tests = 0
custom_stl_test = 1
multi_vof_test = 0


geometry.prob_lo = -11.00 -3.00 -2.25
geometry.prob_hi =  11.00  3.00  2.25

eb2.stl_geometry_method = marching_cubes
eb2.stl_file = B2-9_case_watertight.stl
eb2.stl_scale = 1.0
eb2.stl_center = 0.0 0.0 0.0
eb2.small_volfrac = 1.e-12
eb2.cover_multiple_cuts = 1
eb2.max_grid_size = 32

eb2.eb_surface_stl_file = B2-9_case_singlevof_mc.stl
