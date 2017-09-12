import ciexyz
import colormodels

begin_wl = 360 # hardcoded in get_normalized_spectral_line_colors
end_wl = 830 
delta_wl = 20

xyz_list = ciexyz.get_normalized_spectral_line_colors (brightness=1.0, dwl_angstroms=delta_wl * 10)
xy_list = xyz_list.copy()
(num_colors, num_cols) = xy_list.shape
for i in xrange (0, num_colors):
    colormodels.xyz_normalize (xy_list [i])

entries = len(xy_list)

# print Qt array
print "int begin_wl = {begin_wl};".format(begin_wl=begin_wl)
print "int end_wl = {end_wl};".format(end_wl=end_wl)
print "int delta_wl = {delta_wl};".format(delta_wl=delta_wl)
print "int entries = {entries};".format(entries=entries)
print "qreal monochromatic_xy[{entries}][2] = {{".format(entries=entries)
for xyz in xy_list:
    x = xyz[0]
    y = xyz[1]
    print("    {{ {x:.10f}, {y:.10f} }},").format(x=x, y=y)
print "};"
