# About Polygon Matcher

Polygon Matcher is a C++ plugin for QGIS. It's core is an algorithm matching two polygon rings with respect to an area based quality comparison. The algorithm is shortcutting and running in O(n^3 * m^3) time while consuming O(n^2 * m^2) memory.

# Applications of Polygon Matcher

Polygon Matcher was intended to be used for symmetry detection in building footprints extracted from LIDAR data. The current approach is about to scan building footprint datasets for simple line symmetry within exterior rings themselve. Further targets are symmetry line adjustment and extracting all plane symmetry from the match. Searching for similiar building footprints within datasets or symmetry lines between different footprints are also options.

## Symmetry Line Adjustment

While I have been experiencing symmetry line adjustment with respect to a symmetry line (point and vector) was worst because the shape was turned around the point, and symmetry line adjustment with respect to a vector (symmetry line without point, not fixed, perimeter maximizing) was not accurate (for my mighty), Polygon Matcher offers more independent options for symmetry line adjustment.
