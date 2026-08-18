# B2-9 hull (cartMG Foil/B2-9_case.stl) as a marching-cubes EB case.
#
# The STL is already in metres, yawed 180 deg, trimmed -2.5 deg, heeled 30 deg
# and recentred, so no scaling/translation is applied here.
#   hull bbox : +/-10.374242  +/-2.536116  +/-1.861793  m
#
# B2-9_case.stl itself is NOT watertight (979 open edges across 17 boundary
# loops, one edge shared by four triangles), so the marching-cubes reader
# rejects it.  B2-9_case_watertight.stl is the repaired copy produced by
#   python3 repair_stl.py B2-9_case.stl B2-9_case_watertight.stl 1e-5
# which welds vertices closer than 10 um, drops the resulting slivers and caps
# four tiny leftover holes.  Enclosed volume is unchanged at 88.1633 m^3.
#
# Single level at 4x the cartMG finest resolution (dx = 0.03125 m; level 3 of
# Foil/B2-9.inp is 0.125 m), with the domain shrunk to just enclose the hull:
#   22.0 x 6.0 x 4.5 m  ->  704 x 192 x 144 cells (19.5 M)
# Clearance to the domain boundary is ~20 / ~15 / ~12 cells in x / y / z.

nx = 704
ny = 192
nz = 144

max_grid_size = 32

algorithm_tests = 0
custom_stl_test = 1

geometry.prob_lo = -11.00 -3.00 -2.25
geometry.prob_hi =  11.00  3.00  2.25

eb2.stl_geometry_method = marching_cubes
eb2.stl_file = B2-9_case_watertight.stl
eb2.stl_scale = 1.0
eb2.stl_center = 0.0 0.0 0.0
eb2.small_volfrac = 1.e-12
eb2.cover_multiple_cuts = 1

eb2.mc_stl_file = B2-9_case_mc_triangles.stl
eb2.eb_surface_stl_file = B2-9_case_mc.stl
