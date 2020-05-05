# About Polygon Matcher

Polygon Matcher is a C++ plugin for QGIS. It's core is an algorithm matching two polygon rings with respect to an area based quality comparison. The algorithm is shortcutting and running in O(n^3 * m^3) time while consuming O(n^2 * m^2) memory.

# Applications of Polygon Matcher

Polygon Matcher was intended to be used for symmetry detection in building footprints extracted from LIDAR data. The current approach is about to scan building footprint datasets for simple line symmetry within exterior rings themselve. Further targets are symmetry line adjustment and extracting all plane symmetry from the match. Searching for similiar building footprints within datasets or symmetry lines between different footprints was also an option.

# Symmetry-preserving Generalization of Building Footprints

If you have symmetrical building footprints and want to orthogonalize them with respect to the symmetry line, you can use "Sympector" (http://hagenschwass.name/Sympector). That tool can also simplify with respect to the symmetry lines.
